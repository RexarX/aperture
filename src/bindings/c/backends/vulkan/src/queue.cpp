#include <aperture/vulkan/queue.h>

#include <aperture/queue.h>
#include <aperture/vulkan/header.h>
#include <aperture/vulkan/queue.hpp>

#include "convert.hpp"

#include <stdint.h>

extern "C" {

VkQueue aperture_vk_queue(ApertureQueue queue) noexcept {
  return aperture::vk::GetNative(aperture::cbind::ToCpp(queue)).queue;
}

uint32_t aperture_vk_queue_family(ApertureQueue queue) noexcept {
  return aperture::vk::GetNative(aperture::cbind::ToCpp(queue)).family;
}
}
