#pragma once

#include <aperture/version_macros.h>
#include <aperture/platform.hpp>

#include <compare>
#include <cstdint>

namespace aperture {

/// @brief Semantic library / header version.
/// @details `major` and `minor` are the compatibility key. `patch` is ignored
/// by `Compatible`.
///
/// Rules for header vs linked (DLL / shared object) versions:
/// - Different `major` -> incompatible (ABI / API break).
/// - Same `major`, linked `minor` < header `minor` -> incompatible (headers
///   expect a newer binary).
/// - Same `major`, linked `minor` >= header `minor` -> compatible (newer
///   binary is fine).
struct Version {
  uint8_t major = 0;
  uint8_t minor = 0;
  uint16_t patch = 0;

  [[nodiscard]] constexpr bool operator==(const Version&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const Version&) const noexcept =
      default;
  [[nodiscard]] constexpr auto operator<=>(const Version&) const noexcept =
      default;
};

/// @brief Gets version of the headers visible to this translation unit.
/// @return `Version` struct with `major`, `minor`, and `patch`
[[nodiscard]] constexpr Version HeaderVersion() noexcept {
  return {
      .major = APERTURE_VERSION_MAJOR,
      .minor = APERTURE_VERSION_MINOR,
      .patch = APERTURE_VERSION_PATCH,
  };
}

/// @brief Gets version baked into the linked aperture binary (static lib or
/// DLL).
/// @details Differs from `HeaderVersion()` when an application is compiled
/// against newer/older headers than the loaded shared library.
/// @return `Version` struct containing the linked binary's `major`, `minor`,
/// and `patch`
[[nodiscard]] APERTURE_API Version LinkedVersion() noexcept;

/// @brief Gets version string `"major.minor.patch"` for `LinkedVersion()`.
/// @return Null-terminated version string literal
[[nodiscard]] APERTURE_API const char* VersionString() noexcept;

/// @brief Whether a binary at `linked` can satisfy headers at `header`.
/// @param header Version of the headers the caller compiled against
/// @param linked Version of the aperture binary being used
/// @return `true` if majors match and `linked.minor >= header.minor`, `false`
/// otherwise
[[nodiscard]] constexpr bool Compatible(Version header,
                                        Version linked) noexcept {
  return linked.major == header.major && linked.minor >= header.minor;
}

/// @brief Checks `Compatible(HeaderVersion(), LinkedVersion())`.
/// @return `true` if the linked binary is compatible with the headers, `false`
/// otherwise
[[nodiscard]] inline bool CompatibleWithHeaders() noexcept {
  return Compatible(HeaderVersion(), LinkedVersion());
}

}  // namespace aperture
