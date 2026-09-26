#include <aperture/command/buffer.h>

#include <aperture/command/pool.h>
#include <aperture/result.h>
#include <aperture/types.h>
#include <aperture/assert.hpp>
#include <aperture/command/buffer.hpp>
#include <aperture/command/pool.hpp>
#include <aperture/types.hpp>

#include "convert.hpp"

extern "C" {

ApertureError aperture_begin(ApertureCommandPool pool,
                             ApertureCommandBuffer* out) noexcept {
  APERTURE_ASSERT(out != nullptr);

  auto begun = aperture::Begin(aperture::CommandPool{.ptr = pool});
  if (!begun) [[unlikely]] {
    *out = nullptr;
    return aperture::cbind::ToCError(begun.error());
  }
  *out = static_cast<ApertureCommandBuffer>(begun->ptr);
  return APERTURE_ERROR_OK;
}

void aperture_barrier(ApertureCommandBuffer buffer, ApertureStage src,
                      ApertureStage dst, ApertureHazard hazards) noexcept {
  APERTURE_ASSERT(buffer != nullptr);

  aperture::CommandBuffer token{.ptr = buffer};
  aperture::Barrier(&token, static_cast<aperture::Stage>(src),
                    static_cast<aperture::Stage>(dst),
                    static_cast<aperture::Hazard>(hazards));
}

void aperture_copy(ApertureCommandBuffer buffer, ApertureGpuRange dst,
                   ApertureGpuRange src) noexcept {
  APERTURE_ASSERT(buffer != nullptr);

  aperture::CommandBuffer token{.ptr = buffer};
  aperture::Copy(&token,
                 aperture::GpuRange{
                     .gpu = {.addr = dst.gpu.addr},
                     .size = dst.size,
                 },
                 aperture::GpuRange{
                     .gpu = {.addr = src.gpu.addr},
                     .size = src.size,
                 });
}
}
