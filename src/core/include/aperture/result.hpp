#pragma once

#include <cstdint>
#include <expected>
#include <string_view>

namespace aperture {

/// @brief Recoverable failure of a valid call. Not a programming assert.
enum class Error : uint8_t {
  Ok = 0,
  Unsupported,
  OutOfMemory,
  DeviceLost,
  Invalid,
  OutOfDate,
  Timeout,
};

/// @brief Name of a recoverable `Error` enumerator.
/// @param error Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(Error error) noexcept {
  switch (error) {
    using enum Error;
    case Ok:
      return "Ok";
    case Unsupported:
      return "Unsupported";
    case OutOfMemory:
      return "OutOfMemory";
    case DeviceLost:
      return "DeviceLost";
    case Invalid:
      return "Invalid";
    case OutOfDate:
      return "OutOfDate";
    case Timeout:
      return "Timeout";
  }
  return "Unknown";
}

/// @brief Value-or-error result of a fallible call.
/// @tparam T Payload type on success
template <typename T>
using Result = std::expected<T, Error>;

}  // namespace aperture
