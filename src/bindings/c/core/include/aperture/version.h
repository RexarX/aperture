#ifndef APERTURE_VERSION_H
#define APERTURE_VERSION_H

#include <aperture/platform.h>
#include <aperture/version_macros.h>

#include <stdbool.h>
#include <stdint.h>

APERTURE_C_BEGIN

/// @brief Semantic library / header version.
/// @details Same layout and compatibility rules as `aperture::Version`.
typedef struct ApertureVersion {
  uint8_t major;
  uint8_t minor;
  uint16_t patch;
} ApertureVersion;

/// @brief Gets version of the headers visible to this translation unit.
/// @return `ApertureVersion` with `major`, `minor`, and `patch`
static inline ApertureVersion aperture_header_version(void) {
  const ApertureVersion version = {
      .major = APERTURE_VERSION_MAJOR,
      .minor = APERTURE_VERSION_MINOR,
      .patch = APERTURE_VERSION_PATCH,
  };
  return version;
}

/// @brief Gets version baked into the linked aperture binary (static lib or
/// DLL).
/// @details Differs from `aperture_header_version()` when an application is
/// compiled against newer/older headers than the loaded shared library.
/// @return Linked binary `major`, `minor`, and `patch`
APERTURE_C_API ApertureVersion aperture_linked_version(void)
    APERTURE_C_NOEXCEPT;

/// @brief Gets version string `"major.minor.patch"` for
/// `aperture_linked_version()`.
/// @return Null-terminated version string literal
APERTURE_C_API const char* aperture_version_string(void) APERTURE_C_NOEXCEPT;

/// @brief Whether a binary at `linked` can satisfy headers at `header`.
/// @param header Version of the headers the caller compiled against
/// @param linked Version of the aperture binary being used
/// @return `true` if majors match and `linked.minor >= header.minor`
static inline bool aperture_versions_compatible(ApertureVersion header,
                                                ApertureVersion linked) {
  return linked.major == header.major && linked.minor >= header.minor;
}

/// @brief Checks `aperture_versions_compatible(aperture_header_version(),
///                                             aperture_linked_version())`.
/// @return `true` if the linked binary is compatible with the headers
static inline bool aperture_compatible_with_headers(void) {
  return aperture_versions_compatible(aperture_header_version(),
                                      aperture_linked_version());
}

APERTURE_C_END

#endif
