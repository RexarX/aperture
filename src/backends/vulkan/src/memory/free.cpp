#include <pch.hpp>

#include <aperture/vulkan/memory/free.hpp>

#include <aperture/assert.hpp>
#include <aperture/types.hpp>
#include <aperture/vulkan/memory.hpp>

#include "memory/storage.hpp"

#include <cstddef>
#include <cstdint>

namespace aperture::vk {

void Free(Device* device, DualPtr<std::byte> ptr) noexcept {
  if (ptr.host == nullptr && ptr.device.addr == 0) [[unlikely]] {
    return;
  }
  APERTURE_ASSERT(device != nullptr);
  APERTURE_ASSERT(device->memory != nullptr);

  MemoryState* state = device->memory;
  const HeapGuard guard(state);

  uint64_t addr = ptr.device.addr;
  if (addr == 0) {
    const auto host = state->by_host.find(ptr.host);
    APERTURE_ASSERT(host != state->by_host.end(),
                    "Free of unknown host pointer!");
    addr = host->second;
  }

  const auto found = state->by_device.find(addr);
  APERTURE_ASSERT(found != state->by_device.end(), "Free of unknown DualPtr!");
  const MallocRecord rec = found->second;
  APERTURE_ASSERT(rec.host != nullptr,
                  "Free DualPtr on a Gpu-only allocation!");
  if (ptr.host != nullptr) {
    APERTURE_ASSERT(ptr.host == rec.host, "Free DualPtr host mismatch!");
  }

  Unregister(state, rec);
  rec.block->allocator.Free(rec.span);
  MaybeReleaseDedicated(device, rec.block);
}

void Free(Device* device, GpuPtr<std::byte> ptr) noexcept {
  if (ptr.addr == 0) [[unlikely]] {
    return;
  }
  APERTURE_ASSERT(device != nullptr);
  APERTURE_ASSERT(device->memory != nullptr);

  MemoryState* state = device->memory;
  const HeapGuard guard(state);

  const auto found = state->by_device.find(ptr.addr);
  APERTURE_ASSERT(found != state->by_device.end(), "Free of unknown GpuPtr!");
  const MallocRecord rec = found->second;
  APERTURE_ASSERT(rec.host == nullptr, "Free GpuPtr on a mapped allocation!");

  Unregister(state, rec);
  rec.block->allocator.Free(rec.span);
  MaybeReleaseDedicated(device, rec.block);
}

}  // namespace aperture::vk
