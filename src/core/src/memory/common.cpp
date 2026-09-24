#include <pch.hpp>

#include <aperture/assert.hpp>
#include <aperture/device.hpp>
#include <aperture/types.hpp>

#ifdef APERTURE_HAS_VULKAN
#include <aperture/vulkan/device.hpp>
#include <aperture/vulkan/memory/common.hpp>
#endif

#include <cstddef>

namespace aperture {

auto DeviceAddressOf(Device device, void* host) noexcept -> GpuPtr<std::byte> {
  APERTURE_ASSERT(device.ptr != nullptr);
  APERTURE_ASSERT(host != nullptr);

  switch (BackendOf(device)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      auto* vk_device = static_cast<vk::Device*>(device.ptr);
      return vk::DeviceAddressOf(vk_device, host);
    }
#endif
    default:
      APERTURE_ASSERT(false, "Unsupported backend: {}!",
                      ToString(BackendOf(device)));
      return {};
  }
}

}  // namespace aperture
