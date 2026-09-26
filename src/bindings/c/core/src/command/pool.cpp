#include <aperture/command/pool.h>

#include <aperture/queue.h>
#include <aperture/result.h>
#include <aperture/assert.hpp>
#include <aperture/command/pool.hpp>

#include "convert.hpp"

extern "C" {

ApertureError aperture_create_command_pool(ApertureQueue queue,
                                           const ApertureCommandPoolDesc* desc,
                                           ApertureCommandPool* out) noexcept {
  APERTURE_ASSERT(out != nullptr);

  const aperture::CommandPoolDesc pool_desc{
      .max_timestamps = desc == nullptr ? 0U : desc->max_timestamps,
  };
  auto created =
      aperture::CreateCommandPool(aperture::cbind::ToCpp(queue), pool_desc);
  if (!created) [[unlikely]] {
    *out = nullptr;
    return aperture::cbind::ToCError(created.error());
  }
  *out = static_cast<ApertureCommandPool>(created->ptr);
  return APERTURE_ERROR_OK;
}

void aperture_reset_command_pool(ApertureCommandPool pool) noexcept {
  aperture::Reset(aperture::CommandPool{.ptr = pool});
}

void aperture_destroy_command_pool(ApertureCommandPool pool) noexcept {
  aperture::Destroy(aperture::CommandPool{.ptr = pool});
}
}
