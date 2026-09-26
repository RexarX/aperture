#pragma once

#include <aperture/assert.hpp>
#include <aperture/memory/common.hpp>
#include <aperture/memory/offset_allocator.hpp>
#include <aperture/queue.hpp>
#include <aperture/result.hpp>
#include <aperture/utils/bit.hpp>
#include <aperture/vulkan/device.hpp>
#include <aperture/vulkan/memory/common.hpp>

#include <ankerl/unordered_dense.h>
#include <vk_mem_alloc.h>
#include <volk.h>
#include <aperture/vulkan/header.hpp>

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace aperture::vk {

inline constexpr uint64_t DEFAULT_BLOCK_BYTES = 64ULL * 1024ULL * 1024ULL;
inline constexpr QueueUsage ALL_QUEUE_USAGE =
    QueueUsage::Graphics | QueueUsage::Compute | QueueUsage::Copy;

inline constexpr VkBufferUsageFlags BUFFER_USAGE =
    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
    VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

enum class HeapKind : uint8_t { Default, Gpu, Readback };

[[nodiscard]] constexpr HeapKind KindOf(Memory memory) noexcept {
  switch (memory) {
    using enum Memory;
    case Default:
      return HeapKind::Default;
    case Readback:
      return HeapKind::Readback;
  }
  return HeapKind::Default;
}

[[nodiscard]] constexpr uint32_t MaxAllocsFor(uint32_t bytes) noexcept {
  const uint32_t estimate = (bytes / 16U) + 16U;
  return std::clamp(estimate, 16U, OffsetAllocator::DEFAULT_MAX_ALLOCS);
}

struct Block {
  VkBuffer buffer = VK_NULL_HANDLE;
  VmaAllocation vma = VK_NULL_HANDLE;
  void* mapped = nullptr;
  uint64_t gpu_base = 0;
  uint64_t size = 0;
  OffsetAllocator allocator;
  QueueUsage usage = QueueUsage::Graphics;
  HeapKind kind = HeapKind::Default;
  bool dedicated = false;

  explicit Block(uint32_t bytes) noexcept
      : size(bytes), allocator(bytes, MaxAllocsFor(bytes)) {}
};

struct MallocRecord {
  Block* block = nullptr;
  void* host = nullptr;
  uint64_t device_addr = 0;
  uint64_t size = 0;
  OffsetAllocator::Allocation span;
};

struct Pool {
  std::vector<std::unique_ptr<Block>> blocks;
  HeapKind kind = HeapKind::Default;
  QueueUsage usage = QueueUsage::Graphics;
};

struct MemoryState {
  uint64_t min_align = 16;
  std::vector<Pool> pools;
  ankerl::unordered_dense::map<uint64_t, MallocRecord> by_device;
  ankerl::unordered_dense::map<const void*, uint64_t> by_host;
#ifdef APERTURE_ENABLE_VALIDATION_SUPPORT
  std::atomic_flag busy;
#endif
};

#ifdef APERTURE_ENABLE_VALIDATION_SUPPORT
/// @brief Asserts if another heap op is already in progress on `state`.
/// @details Not a lock: release builds with validation OFF compile this away.
struct HeapGuard {
  MemoryState* state = nullptr;

  explicit HeapGuard(MemoryState* memory) noexcept : state(memory) {
    APERTURE_ASSERT(state != nullptr);
    const bool busy = state->busy.test_and_set(std::memory_order_acquire);
    APERTURE_ASSERT(!busy,
                    "Concurrent Malloc/Free/DeviceAddressOf on one Device!");
    if (busy) {
      state = nullptr;
    }
  }
  HeapGuard(const HeapGuard&) = delete;
  HeapGuard(HeapGuard&&) = delete;
  ~HeapGuard() noexcept {
    if (state != nullptr) {
      state->busy.clear(std::memory_order_release);
    }
  }

  HeapGuard& operator=(const HeapGuard&) = delete;
  HeapGuard& operator=(HeapGuard&&) = delete;
};
#else
struct HeapGuard {
  explicit HeapGuard(MemoryState*) noexcept {}
};
#endif

inline void Register(MemoryState* state, const MallocRecord& rec) noexcept {
  APERTURE_ASSERT(state != nullptr);

  state->by_device.emplace(rec.device_addr, rec);
  if (rec.host != nullptr) {
    state->by_host.emplace(rec.host, rec.device_addr);
  }
}

inline void Unregister(MemoryState* state, const MallocRecord& rec) noexcept {
  APERTURE_ASSERT(state != nullptr);

  state->by_device.erase(rec.device_addr);
  if (rec.host != nullptr) {
    state->by_host.erase(rec.host);
  }
}

/// @brief Spanning buffer that contains `ptr`, plus the allocation's queues.
struct ResolvedRange {
  VkBuffer buffer = VK_NULL_HANDLE;
  uint64_t offset = 0;
  uint64_t remaining = 0;
  QueueUsage usage = QueueUsage::Graphics;
};

/// @brief Allocation that contains `ptr`.
/// @param device Device that owns the heap
/// @param ptr Device address, or an offset into an allocation
/// @return Buffer, byte offset, bytes remaining, and queue mask
/// @warning Asserts if `device` is null, `ptr` is 0, or `ptr` is unknown.
[[nodiscard]] ResolvedRange FindRange(Device* device,
                                      GpuPtr<std::byte> ptr) noexcept;

[[nodiscard]] inline Buffer BufferFromBlock(const Block& block,
                                            uint64_t address,
                                            uint64_t size) noexcept {
  return {
      .buffer = block.buffer,
      .address = address,
      .offset = address - block.gpu_base,
      .size = size,
  };
}

void InitMemory(Device* device) noexcept;
void DestroyMemory(Device* device) noexcept;

[[nodiscard]] auto ValidateUsage(const Device& device, QueueUsage usage,
                                 bool log) noexcept -> Result<void>;

[[nodiscard]] auto CreateBlock(Device* device, HeapKind kind, QueueUsage usage,
                               uint64_t size, bool dedicated) noexcept
    -> Result<Block*>;
void DestroyBlock(Device* device, Block* block) noexcept;
void MaybeReleaseDedicated(Device* device, Block* block) noexcept;

[[nodiscard]] auto AllocateFromHeaps(Device* device, HeapKind kind,
                                     QueueUsage usage, size_t bytes,
                                     size_t align, bool dedicated,
                                     bool log) noexcept -> Result<MallocRecord>;

}  // namespace aperture::vk
