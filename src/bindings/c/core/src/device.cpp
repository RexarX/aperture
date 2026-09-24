#include <aperture/device.h>

#include <aperture/capability.h>
#include <aperture/instance.h>
#include <aperture/assert.hpp>
#include <aperture/capability.hpp>
#include <aperture/device.hpp>

#include "convert.hpp"

extern "C" {

ApertureBackend aperture_backend_of_device(ApertureDevice device) noexcept {
  return aperture::cbind::ToCBackend(
      aperture::BackendOf(aperture::cbind::ToCpp(device)));
}

ApertureError aperture_create_device(ApertureInstance instance,
                                     const ApertureDeviceDesc* desc,
                                     ApertureDevice* out) noexcept {
  APERTURE_ASSERT(desc != nullptr);
  APERTURE_ASSERT(out != nullptr);
  *out = nullptr;
  auto result = aperture::CreateDevice(aperture::cbind::ToCpp(instance),
                                       aperture::cbind::ToCpp(*desc));
  if (!result) [[unlikely]] {
    return aperture::cbind::ToCError(result.error());
  }
  *out = aperture::cbind::ToC(*result);
  return APERTURE_ERROR_OK;
}

void aperture_destroy_device(ApertureDevice device) noexcept {
  aperture::Destroy(aperture::cbind::ToCpp(device));
}

bool aperture_device_has(ApertureDevice device,
                         ApertureCapability caps) noexcept {
  return aperture::Has(aperture::cbind::ToCpp(device),
                       static_cast<aperture::Capability>(caps));
}

ApertureDeviceInfo aperture_device_info(ApertureDevice device) noexcept {
  return aperture::cbind::ToC(aperture::Info(aperture::cbind::ToCpp(device)));
}
}
