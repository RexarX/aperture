#include <aperture/memory/free.h>

#include <aperture/device.h>
#include <aperture/types.h>
#include <aperture/memory/free.hpp>

#include "convert.hpp"

#include <cstddef>

extern "C" {

void aperture_free(ApertureDevice device, ApertureDualPtr ptr) noexcept {
  aperture::Free(aperture::cbind::ToCpp(device),
                 {
                     .host = static_cast<std::byte*>(ptr.host),
                     .device = {.addr = ptr.device.addr},
                 });
}

void aperture_free_gpu(ApertureDevice device, ApertureGpuPtr ptr) noexcept {
  aperture::Free(aperture::cbind::ToCpp(device), {.addr = ptr.addr});
}
}
