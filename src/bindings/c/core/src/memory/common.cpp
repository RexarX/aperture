#include <aperture/memory/common.h>

#include <aperture/device.h>
#include <aperture/types.h>
#include <aperture/memory/common.hpp>

#include "convert.hpp"

extern "C" {

ApertureGpuPtr aperture_device_address_of(ApertureDevice device,
                                          void* host) noexcept {
  return aperture::cbind::ToC(
      aperture::DeviceAddressOf(aperture::cbind::ToCpp(device), host));
}
}
