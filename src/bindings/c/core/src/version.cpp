#include <aperture/version.h>

#include <aperture/version.hpp>

#include "convert.hpp"

extern "C" {

ApertureVersion aperture_linked_version(void) noexcept {
  return aperture::cbind::ToC(aperture::LinkedVersion());
}

const char* aperture_version_string(void) noexcept {
  return aperture::VersionString();
}
}
