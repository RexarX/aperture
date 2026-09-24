#include <pch.hpp>

#include <aperture/memory/free.hpp>

#include <aperture/assert.hpp>
#include <aperture/device.hpp>
#include <aperture/types.hpp>

#ifdef APERTURE_HAS_VULKAN
#include <aperture/vulkan/memory/free.hpp>
#endif

#include <cstddef>

namespace aperture {

void Free(Device device, DualPtr<std::byte> ptr) noexcept {
  if (ptr.host == nullptr && ptr.device.addr == 0) [[unlikely]] {
    return;
  }
  APERTURE_ASSERT(device.ptr != nullptr);

  switch (BackendOf(device)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      auto* vk_device = static_cast<vk::Device*>(device.ptr);
      vk::Free(vk_device, ptr);
      return;
    }
#endif
    default:
      APERTURE_ASSERT(false, "Unsupported backend: {}!",
                      ToString(BackendOf(device)));
  }
}

void Free(Device device, GpuPtr<std::byte> ptr) noexcept {
  if (ptr.addr == 0) [[unlikely]] {
    return;
  }
  APERTURE_ASSERT(device.ptr != nullptr);

  switch (BackendOf(device)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      auto* vk_device = static_cast<vk::Device*>(device.ptr);
      vk::Free(vk_device, ptr);
      return;
    }
#endif
    default:
      APERTURE_ASSERT(false, "Unsupported backend: {}!",
                      ToString(BackendOf(device)));
  }
}

}  // namespace aperture
