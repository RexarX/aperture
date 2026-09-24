#include <pch.hpp>

#include "memory/storage.hpp"

#include <aperture/assert.hpp>
#include <aperture/memory/offset_allocator.hpp>
#include <aperture/queue.hpp>
#include <aperture/result.hpp>
#include <aperture/utils/bit.hpp>
#include <aperture/vulkan/device.hpp>

#include "internal.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <memory>
#include <utility>

namespace aperture::vk {

namespace {

void FillQueueFamilies(const Device& device, QueueUsage usage,
                       std::array<uint32_t, 3>* families,
                       uint32_t* count) noexcept {
  APERTURE_ASSERT(families != nullptr);
  APERTURE_ASSERT(count != nullptr);

  *count = 0;
  auto add = [families, count](uint32_t family) noexcept {
    for (uint32_t i = 0; i < *count; ++i) {
      if ((*families)[i] == family) {
        return;
      }
    }
    (*families)[(*count)++] = family;
  };

  if (HasAll(usage, QueueUsage::Graphics)) {
    add(device.graphics.family);
  }
  if (HasAll(usage, QueueUsage::Compute)) {
    add(device.compute.family);
  }
  if (HasAll(usage, QueueUsage::Copy)) {
    add(device.copy.family);
  }
}

void FillAllocInfo(HeapKind kind, VmaAllocationCreateInfo* info) noexcept {
  APERTURE_ASSERT(info != nullptr);

  *info = {};
  switch (kind) {
    using enum HeapKind;
    case Default:
      info->flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                    VMA_ALLOCATION_CREATE_MAPPED_BIT;
      info->usage = VMA_MEMORY_USAGE_AUTO;
      info->requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      info->preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      break;
    case Gpu:
      info->usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
      info->requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      break;
    case Readback:
      info->flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |
                    VMA_ALLOCATION_CREATE_MAPPED_BIT;
      info->usage = VMA_MEMORY_USAGE_AUTO;
      info->requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                            VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
      break;
  }
}

[[nodiscard]] auto TrySuballoc(Block* block, uint32_t bytes) noexcept
    -> std::optional<OffsetAllocator::Allocation> {
  APERTURE_ASSERT(block != nullptr);

  auto span = block->allocator.Allocate(bytes);
  if (!span) [[unlikely]] {
    return std::nullopt;
  }
  return span;
}

[[nodiscard]] constexpr uint64_t PlacementAlign(const MemoryState& state,
                                                size_t align) noexcept {
  return std::max(state.min_align, static_cast<uint64_t>(align));
}

[[nodiscard]] constexpr uint64_t SuballocBytes(uint64_t nbytes,
                                               uint64_t place_align,
                                               uint64_t min_align) noexcept {
  const uint64_t rounded = utils::AlignUp(nbytes, place_align);
  if (place_align <= min_align) {
    return rounded;
  }
  return rounded + (place_align - min_align);
}

}  // namespace

void InitMemory(Device* device) noexcept {
  APERTURE_ASSERT(device != nullptr);
  APERTURE_ASSERT(device->memory == nullptr);

  auto* state = new MemoryState{};
  state->pools.reserve(8);
  VkPhysicalDeviceProperties props{};
  vkGetPhysicalDeviceProperties(device->physical_device, &props);
  state->min_align =
      std::max<uint64_t>(16, props.limits.minStorageBufferOffsetAlignment);
  device->memory = state;
}

void DestroyMemory(Device* device) noexcept {
  APERTURE_ASSERT(device != nullptr);

  MemoryState* state = device->memory;
  if (state == nullptr) {
    return;
  }

  for (Pool& pool : state->pools) {
    for (auto& owned : pool.blocks) {
      DestroyBlock(device, owned.get());
    }
    pool.blocks.clear();
  }
  state->pools.clear();
  state->by_device.clear();
  state->by_host.clear();

  delete state;
  device->memory = nullptr;
}

auto ValidateUsage(const Device& device, QueueUsage usage, bool log) noexcept
    -> Result<void> {
  const auto bits = std::to_underlying(usage);
  const auto allowed = std::to_underlying(ALL_QUEUE_USAGE);
  if (bits == 0 || (bits & ~allowed) != 0) [[unlikely]] {
    LogFailed(log, "Malloc QueueUsage is empty or has unknown bits ({})!",
              ToString(Error::Invalid));
    return std::unexpected(Error::Invalid);
  }
  if (HasAll(usage, QueueUsage::Compute) && !device.has_compute) [[unlikely]] {
    LogFailed(log, "Malloc requested compute queue usage ({})!",
              ToString(Error::Unsupported));
    return std::unexpected(Error::Unsupported);
  }
  if (HasAll(usage, QueueUsage::Copy) && !device.has_copy) [[unlikely]] {
    LogFailed(log, "Malloc requested copy queue usage ({})!",
              ToString(Error::Unsupported));
    return std::unexpected(Error::Unsupported);
  }
  return {};
}

auto CreateBlock(Device* device, HeapKind kind, QueueUsage usage, uint64_t size,
                 bool dedicated) noexcept -> Result<Block*> {
  APERTURE_ASSERT(device != nullptr);
  APERTURE_ASSERT(device->memory != nullptr);
  APERTURE_ASSERT(size != 0);
  if (size > (std::numeric_limits<uint32_t>::max)()) [[unlikely]] {
    return std::unexpected(Error::OutOfMemory);
  }

  const auto bytes = static_cast<uint32_t>(size);

  std::array<uint32_t, 3> families = {};
  uint32_t family_count = 0;
  FillQueueFamilies(*device, usage, &families, &family_count);
  APERTURE_ASSERT(family_count != 0);

  VkBufferCreateInfo buffer_ci{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .flags = device->capture_replay
                   ? VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT
                   : VkBufferCreateFlags{},
      .size = size,
      .usage = BUFFER_USAGE,
      .sharingMode = family_count > 1 ? VK_SHARING_MODE_CONCURRENT
                                      : VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = family_count > 1 ? family_count : 0,
      .pQueueFamilyIndices = family_count > 1 ? families.data() : nullptr,
  };

  VmaAllocationCreateInfo alloc_ci{};
  FillAllocInfo(kind, &alloc_ci);
  alloc_ci.minAlignment = device->memory->min_align;

  VkBuffer buffer = VK_NULL_HANDLE;
  VmaAllocation vma = VK_NULL_HANDLE;
  VmaAllocationInfo vma_info{};
  const VkResult created = vmaCreateBuffer(device->allocator, &buffer_ci,
                                           &alloc_ci, &buffer, &vma, &vma_info);
  if (created != VK_SUCCESS) [[unlikely]] {
    return std::unexpected(MapVkResult(created));
  }
  if ((kind == HeapKind::Default || kind == HeapKind::Readback) &&
      vma_info.pMappedData == nullptr) [[unlikely]] {
    vmaDestroyBuffer(device->allocator, buffer, vma);
    return std::unexpected(Error::OutOfMemory);
  }

  const VkBufferDeviceAddressInfo addr_info{
      .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
      .buffer = buffer,
  };
  const uint64_t gpu_base =
      vkGetBufferDeviceAddress(device->device, &addr_info);

  MemoryState* state = device->memory;
  auto pool_it =
      std::ranges::find_if(state->pools, [kind, usage](const Pool& pool) {
        return pool.kind == kind && pool.usage == usage;
      });
  if (pool_it == state->pools.end()) {
    state->pools.push_back({.kind = kind, .usage = usage});
    pool_it = state->pools.end() - 1;
  }

  auto block = std::make_unique<Block>(bytes);
  block->buffer = buffer;
  block->vma = vma;
  block->mapped = vma_info.pMappedData;
  block->gpu_base = gpu_base;
  block->usage = usage;
  block->kind = kind;
  block->dedicated = dedicated;
  Block* raw = block.get();
  pool_it->blocks.push_back(std::move(block));
  return raw;
}

void DestroyBlock(Device* device, Block* block) noexcept {
  APERTURE_ASSERT(device != nullptr);
  APERTURE_ASSERT(block != nullptr);

  if (block->buffer != VK_NULL_HANDLE) {
    vmaDestroyBuffer(device->allocator, block->buffer, block->vma);
    block->buffer = VK_NULL_HANDLE;
    block->vma = VK_NULL_HANDLE;
  }
}

void MaybeReleaseDedicated(Device* device, Block* block) noexcept {
  APERTURE_ASSERT(device != nullptr);
  APERTURE_ASSERT(block != nullptr);
  APERTURE_ASSERT(device->memory != nullptr);
  if (!block->dedicated && block->size <= DEFAULT_BLOCK_BYTES) [[likely]] {
    return;
  }
  if (!block->allocator.Empty()) [[unlikely]] {
    return;
  }

  MemoryState* state = device->memory;
  for (Pool& pool : state->pools) {
    const auto found = std::ranges::find_if(
        pool.blocks,
        [block](const auto& owned) { return owned.get() == block; });
    if (found == pool.blocks.end()) {
      continue;
    }
    DestroyBlock(device, block);
    pool.blocks.erase(found);
    return;
  }
}

auto AllocateFromHeaps(Device* device, HeapKind kind, QueueUsage usage,
                       size_t bytes, size_t align, bool dedicated,
                       bool log) noexcept -> Result<MallocRecord> {
  APERTURE_ASSERT(device != nullptr);
  APERTURE_ASSERT(device->memory != nullptr);

  MemoryState* state = device->memory;
  const HeapGuard guard(state);

  const uint64_t place_align = PlacementAlign(*state, align);
  const auto nbytes = static_cast<uint64_t>(bytes);
  const uint64_t needed = SuballocBytes(nbytes, place_align, state->min_align);
  if (needed > (std::numeric_limits<uint32_t>::max)()) [[unlikely]] {
    LogFailed(log, "Malloc size exceeds OffsetAllocator range ({})!",
              ToString(Error::OutOfMemory));
    return std::unexpected(Error::OutOfMemory);
  }
  const auto request = static_cast<uint32_t>(needed);

  auto make_record = [place_align, nbytes, request](
                         Block* block,
                         OffsetAllocator::Allocation span) noexcept {
    const uint64_t raw = block->gpu_base + static_cast<uint64_t>(span.offset);
    const uint64_t aligned = utils::AlignUp(raw, place_align);
    APERTURE_ASSERT(aligned + nbytes <= raw + request);
    const uint64_t local = aligned - block->gpu_base;
    return MallocRecord{
        .block = block,
        .host = block->mapped != nullptr
                    ? static_cast<std::byte*>(block->mapped) + local
                    : nullptr,
        .device_addr = aligned,
        .size = nbytes,
        .span = span,
    };
  };

  if (!dedicated) {
    for (Pool& pool : state->pools) {
      if (pool.kind != kind || pool.usage != usage) {
        continue;
      }
      for (auto& owned : pool.blocks) {
        Block* block = owned.get();
        if (block->dedicated) {
          continue;
        }
        const auto span = TrySuballoc(block, request);
        if (!span) {
          continue;
        }
        MallocRecord rec = make_record(block, *span);
        Register(state, rec);
        return rec;
      }
    }
  }

  const uint64_t block_size =
      dedicated ? needed : std::max(DEFAULT_BLOCK_BYTES, needed);
  const auto created = CreateBlock(device, kind, usage, block_size, dedicated);
  if (!created) [[unlikely]] {
    LogFailed(log, "Failed to create memory block of {} bytes ({})!",
              block_size, ToString(created.error()));
    return std::unexpected(created.error());
  }

  const auto span = TrySuballoc(*created, request);
  if (!span) [[unlikely]] {
    Block* block = *created;
    for (Pool& pool : state->pools) {
      const auto found = std::ranges::find_if(
          pool.blocks,
          [block](const auto& owned) { return owned.get() == block; });
      if (found == pool.blocks.end()) {
        continue;
      }
      DestroyBlock(device, block);
      pool.blocks.erase(found);
      break;
    }
    LogFailed(log, "Suballoc failed after creating a new block ({})!",
              ToString(Error::OutOfMemory));
    return std::unexpected(Error::OutOfMemory);
  }
  MallocRecord rec = make_record(*created, *span);
  Register(state, rec);
  return rec;
}

}  // namespace aperture::vk
