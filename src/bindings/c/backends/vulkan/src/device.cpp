#include <aperture/vulkan/device.h>

#include <aperture/device.h>
#include <aperture/instance.h>
#include <aperture/result.h>
#include <aperture/vulkan/header.h>
#include <aperture/assert.hpp>
#include <aperture/device.hpp>
#include <aperture/result.hpp>
#include <aperture/vulkan/device.hpp>
#include <aperture/vulkan/instance.hpp>

#include "convert.hpp"

#include <stddef.h>

extern "C" {

ApertureError aperture_vk_create_device(ApertureInstance instance,
                                        const ApertureDeviceDesc* desc,
                                        const ApertureVkDeviceExtras* extras,
                                        ApertureDevice* out) noexcept {
  APERTURE_ASSERT(desc != nullptr);
  APERTURE_ASSERT(out != nullptr);
  *out = nullptr;

  aperture::vk::Instance& vk_instance =
      aperture::vk::GetNative(aperture::cbind::ToCpp(instance));
  const aperture::DeviceDesc cpp_desc = aperture::cbind::ToCpp(*desc);

  aperture::Result<aperture::vk::Device*> created;
  if (extras == nullptr) {
    created = aperture::vk::CreateDevice(&vk_instance, cpp_desc);
  } else {
    const aperture::vk::DeviceExtras cpp_extras{
        .extensions = {extras->extensions, extras->extension_count},
        .features = extras->features,
    };
    created = aperture::vk::CreateDevice(&vk_instance, cpp_desc, cpp_extras);
  }
  if (!created) [[unlikely]] {
    return aperture::cbind::ToCError(created.error());
  }
  *out = aperture::cbind::ToC(aperture::Device{.ptr = *created});
  return APERTURE_ERROR_OK;
}

VkDevice aperture_vk_device(ApertureDevice device) noexcept {
  return aperture::vk::GetNative(aperture::cbind::ToCpp(device)).device;
}

VkPhysicalDevice aperture_vk_physical_device(ApertureDevice device) noexcept {
  return aperture::vk::GetNative(aperture::cbind::ToCpp(device))
      .physical_device;
}

VkInstance aperture_vk_device_instance(ApertureDevice device) noexcept {
  return aperture::vk::GetNative(aperture::cbind::ToCpp(device)).instance;
}
}
