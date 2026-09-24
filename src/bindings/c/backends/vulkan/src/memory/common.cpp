#include <aperture/vulkan/memory/common.h>

#include <aperture/device.h>
#include <aperture/types.h>
#include <aperture/vulkan/memory/common.hpp>

#include "convert.hpp"

extern "C" {

ApertureVkBuffer aperture_vk_buffer(ApertureDevice device,
                                    ApertureGpuPtr ptr) noexcept {
  const aperture::vk::Buffer buffer = aperture::vk::GetVkBuffer(
      aperture::cbind::ToCpp(device), aperture::cbind::ToCpp(ptr));
  return {
      .buffer = buffer.buffer,
      .address = buffer.address,
      .offset = buffer.offset,
      .size = buffer.size,
  };
}
}
