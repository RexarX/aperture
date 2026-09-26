#include <aperture/adapter.h>

#include <aperture/format.h>
#include <aperture/surface.h>
#include <aperture/adapter.hpp>
#include <aperture/assert.hpp>
#include <aperture/format.hpp>

#include "convert.hpp"

#include <cstddef>

extern "C" {

bool aperture_supports_format(const ApertureAdapter* adapter,
                              ApertureTextureFormat format,
                              ApertureFormatUsage usage) noexcept {
  APERTURE_ASSERT(adapter != nullptr);

  aperture::Adapter cpp{};
  cpp.impl = adapter->impl;
  return aperture::SupportsFormat(cpp,
                                  static_cast<aperture::TextureFormat>(format),
                                  static_cast<aperture::FormatUsage>(usage));
}

void aperture_presentable_formats(const ApertureAdapter* adapter,
                                  const ApertureSurface* surface,
                                  const ApertureTextureFormat** data,
                                  size_t* size) noexcept {
  APERTURE_ASSERT(adapter != nullptr);
  APERTURE_ASSERT(surface != nullptr);
  APERTURE_ASSERT(data != nullptr);
  APERTURE_ASSERT(size != nullptr);

  aperture::Adapter cpp{};
  cpp.impl = adapter->impl;
  const auto formats =
      aperture::PresentableFormats(cpp, aperture::cbind::ToCpp(*surface));
  *data = reinterpret_cast<const ApertureTextureFormat*>(formats.data());
  *size = formats.size();
}
}
