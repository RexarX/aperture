#include <aperture/aperture.h>

#include <aperture/aperture.hpp>

extern "C" const char* aperture_version(void) noexcept {
  return APERTURE_PROJECT_VERSION;
}
