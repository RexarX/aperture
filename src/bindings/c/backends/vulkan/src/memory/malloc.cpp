#include <aperture/vulkan/memory/malloc.h>

#include <aperture/device.h>
#include <aperture/memory/common.h>
#include <aperture/memory/malloc.h>
#include <aperture/queue.h>
#include <aperture/result.h>
#include <aperture/types.h>
#include <aperture/assert.hpp>
#include <aperture/memory/common.hpp>
#include <aperture/memory/malloc.hpp>
#include <aperture/queue.hpp>
#include <aperture/vulkan/device.hpp>
#include <aperture/vulkan/memory/malloc.hpp>

#include "convert.hpp"

#include <cstddef>

extern "C" {

ApertureError aperture_vk_malloc(ApertureDevice device, size_t bytes,
                                 size_t align, ApertureMemory memory,
                                 ApertureQueueUsage usage,
                                 ApertureMallocFlags flags,
                                 ApertureDualPtr* out) noexcept {
  APERTURE_ASSERT(out != nullptr);
  aperture::vk::Device& vk_device =
      aperture::vk::GetNative(aperture::cbind::ToCpp(device));
  auto result = aperture::vk::Malloc(&vk_device, bytes, align,
                                     static_cast<aperture::Memory>(memory),
                                     static_cast<aperture::QueueUsage>(usage),
                                     static_cast<aperture::MallocFlags>(flags));
  if (!result) [[unlikely]] {
    *out = {};
    return aperture::cbind::ToCError(result.error());
  }
  *out = aperture::cbind::ToC(*result);
  return APERTURE_ERROR_OK;
}

ApertureError aperture_vk_malloc_gpu(ApertureDevice device, size_t bytes,
                                     size_t align, ApertureQueueUsage usage,
                                     ApertureMallocFlags flags,
                                     ApertureGpuPtr* out) noexcept {
  APERTURE_ASSERT(out != nullptr);
  aperture::vk::Device& vk_device =
      aperture::vk::GetNative(aperture::cbind::ToCpp(device));
  auto result = aperture::vk::MallocGpu(
      &vk_device, bytes, align, static_cast<aperture::QueueUsage>(usage),
      static_cast<aperture::MallocFlags>(flags));
  if (!result) [[unlikely]] {
    *out = {};
    return aperture::cbind::ToCError(result.error());
  }
  *out = aperture::cbind::ToC(*result);
  return APERTURE_ERROR_OK;
}
}
