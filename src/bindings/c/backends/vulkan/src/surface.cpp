#include <aperture/vulkan/surface.h>

#include <aperture/instance.h>
#include <aperture/result.h>
#include <aperture/surface.h>
#include <aperture/assert.hpp>
#include <aperture/vulkan/instance.hpp>
#include <aperture/vulkan/surface.hpp>

#include "convert.hpp"

extern "C" {

ApertureError aperture_vk_create_surface(ApertureInstance instance,
                                         const ApertureSurface* surface,
                                         ApertureVkSurface* out) noexcept {
  APERTURE_ASSERT(surface != nullptr);
  APERTURE_ASSERT(out != nullptr);
  *out = {};
  aperture::vk::Instance& vk_instance =
      aperture::vk::GetNative(aperture::cbind::ToCpp(instance));
  auto created = aperture::vk::CreateSurface(&vk_instance,
                                             aperture::cbind::ToCpp(*surface));
  if (!created) [[unlikely]] {
    return aperture::cbind::ToCError(created.error());
  }
  out->surface = created->surface;
  return APERTURE_ERROR_OK;
}

void aperture_vk_destroy_surface(ApertureInstance instance,
                                 ApertureVkSurface surface) noexcept {
  aperture::vk::Instance& vk_instance =
      aperture::vk::GetNative(aperture::cbind::ToCpp(instance));
  aperture::vk::DestroySurface(&vk_instance, {.surface = surface.surface});
}
}
