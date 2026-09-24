#include <aperture/queue.h>

#include <aperture/device.h>
#include <aperture/result.h>
#include <aperture/assert.hpp>
#include <aperture/queue.hpp>

#include "convert.hpp"

extern "C" {

ApertureQueue aperture_graphics_queue(ApertureDevice device) noexcept {
  return aperture::cbind::ToC(
      aperture::GraphicsQueue(aperture::cbind::ToCpp(device)));
}

ApertureError aperture_compute_queue(ApertureDevice device,
                                     ApertureQueue* out) noexcept {
  APERTURE_ASSERT(out != nullptr);
  auto result = aperture::ComputeQueue(aperture::cbind::ToCpp(device));
  if (!result) [[unlikely]] {
    *out = nullptr;
    return aperture::cbind::ToCError(result.error());
  }
  *out = aperture::cbind::ToC(*result);
  return APERTURE_ERROR_OK;
}

ApertureError aperture_copy_queue(ApertureDevice device,
                                  ApertureQueue* out) noexcept {
  APERTURE_ASSERT(out != nullptr);
  auto result = aperture::CopyQueue(aperture::cbind::ToCpp(device));
  if (!result) [[unlikely]] {
    *out = nullptr;
    return aperture::cbind::ToCError(result.error());
  }
  *out = aperture::cbind::ToC(*result);
  return APERTURE_ERROR_OK;
}
}
