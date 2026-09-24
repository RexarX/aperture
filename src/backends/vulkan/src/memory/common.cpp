#include <pch.hpp>

#include <aperture/vulkan/memory/common.hpp>

#include <aperture/assert.hpp>
#include <aperture/device.hpp>
#include <aperture/types.hpp>

#include "memory/storage.hpp"

#include <cstddef>
#include <cstdint>

namespace aperture::vk {

auto DeviceAddressOf(Device* device, void* host) noexcept -> GpuPtr<std::byte> {
  APERTURE_ASSERT(device != nullptr);
  APERTURE_ASSERT(host != nullptr);
  APERTURE_ASSERT(device->memory != nullptr);

  MemoryState* state = device->memory;
  const HeapGuard guard(state);

  const auto found = state->by_host.find(host);
  APERTURE_ASSERT(found != state->by_host.end(), "Unknown host pointer!");
  return {.addr = found->second};
}

Buffer GetVkBuffer(aperture::Device device, GpuPtr<std::byte> ptr) noexcept {
  APERTURE_ASSERT(device.ptr != nullptr);
  auto* impl = static_cast<Device*>(device.ptr);
  APERTURE_ASSERT(impl->backend == Backend::Vulkan);
  APERTURE_ASSERT(impl->memory != nullptr);
  APERTURE_ASSERT(ptr.addr != 0);

  MemoryState* state = impl->memory;
  const HeapGuard guard(state);

  const MallocRecord* match = nullptr;
  if (const auto found = state->by_device.find(ptr.addr);
      found != state->by_device.end()) {
    match = &found->second;
  } else {
    for (const auto& entry : state->by_device) {
      const MallocRecord& rec = entry.second;
      if (ptr.addr >= rec.device_addr &&
          ptr.addr < rec.device_addr + rec.size) {
        match = &rec;
        break;
      }
    }
  }
  if (match != nullptr) {
    return BufferFromBlock(*match->block, ptr.addr,
                           match->device_addr + match->size - ptr.addr);
  }

  APERTURE_ASSERT(false, "Unknown GpuPtr!");
  return {};
}

}  // namespace aperture::vk
