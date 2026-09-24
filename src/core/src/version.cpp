#include <pch.hpp>

#include <aperture/version.hpp>

namespace aperture {

Version LinkedVersion() noexcept {
  return HeaderVersion();
}

const char* VersionString() noexcept {
  return APERTURE_VERSION_STRING;
}

}  // namespace aperture
