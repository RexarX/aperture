#include <aperture/vulkan/memory/free.h>

#include <aperture/device.h>
#include <aperture/types.h>
#include <aperture/vulkan/device.hpp>
#include <aperture/vulkan/memory/free.hpp>

#include "convert.hpp"

#include <cstddef>

extern "C" {

void aperture_vk_free(ApertureDevice device, ApertureDualPtr ptr) noexcept {
  aperture::vk::Device& vk_device =
      aperture::vk::GetNative(aperture::cbind::ToCpp(device));
  aperture::vk::Free(&vk_device, {
                                     .host = static_cast<std::byte*>(ptr.host),
                                     .device = {.addr = ptr.device.addr},
                                 });
}

void aperture_vk_free_gpu(ApertureDevice device, ApertureGpuPtr ptr) noexcept {
  aperture::vk::Device& vk_device =
      aperture::vk::GetNative(aperture::cbind::ToCpp(device));
  aperture::vk::Free(&vk_device, {.addr = ptr.addr});
}
}
