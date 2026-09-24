#include <aperture/vulkan/adapter.h>

#include <aperture/adapter.h>
#include <aperture/adapter.hpp>
#include <aperture/assert.hpp>
#include <aperture/vulkan/adapter.hpp>

#include <stdbool.h>
#include <stddef.h>

namespace {

[[nodiscard]] aperture::Adapter PortableAdapter(
    const ApertureAdapter* adapter) noexcept {
  APERTURE_ASSERT(adapter != nullptr);
  aperture::Adapter cpp{};
  cpp.impl = adapter->impl;
  return cpp;
}

}  // namespace

extern "C" {

VkPhysicalDevice aperture_vk_adapter_physical_device(
    const ApertureAdapter* adapter) noexcept {
  return aperture::vk::PhysicalDevice(
      aperture::vk::GetNative(PortableAdapter(adapter)));
}

bool aperture_vk_has_device_extension(const ApertureAdapter* adapter,
                                      const char* name) noexcept {
  APERTURE_ASSERT(name != nullptr);
  return aperture::vk::HasDeviceExtension(
      aperture::vk::GetNative(PortableAdapter(adapter)), name);
}

void aperture_vk_device_extensions(const ApertureAdapter* adapter,
                                   const VkExtensionProperties** data,
                                   size_t* size) noexcept {
  APERTURE_ASSERT(data != nullptr);
  APERTURE_ASSERT(size != nullptr);
  const auto extensions = aperture::vk::DeviceExtensions(
      aperture::vk::GetNative(PortableAdapter(adapter)));
  *data = extensions.data();
  *size = extensions.size();
}
}
