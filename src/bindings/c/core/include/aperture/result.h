#ifndef APERTURE_RESULT_H
#define APERTURE_RESULT_H

#include <stdint.h>

/// @brief Recoverable failure of a valid call. Not a programming assert.
/// @details Stored as `uint8_t` so C enums do not widen to `int`.
typedef uint8_t ApertureError;

enum {
  APERTURE_ERROR_OK = 0U,
  APERTURE_ERROR_UNSUPPORTED,
  APERTURE_ERROR_OUT_OF_MEMORY,
  APERTURE_ERROR_DEVICE_LOST,
  APERTURE_ERROR_INVALID,
  APERTURE_ERROR_OUT_OF_DATE,
  APERTURE_ERROR_TIMEOUT,
  APERTURE_ERROR_VERSION_MISMATCH,
};

/// @brief Name of a recoverable `ApertureError` enumerator.
/// @param error Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
static inline const char* aperture_error_to_string(ApertureError error) {
  switch (error) {
    case APERTURE_ERROR_OK:
      return "Ok";
    case APERTURE_ERROR_UNSUPPORTED:
      return "Unsupported";
    case APERTURE_ERROR_OUT_OF_MEMORY:
      return "OutOfMemory";
    case APERTURE_ERROR_DEVICE_LOST:
      return "DeviceLost";
    case APERTURE_ERROR_INVALID:
      return "Invalid";
    case APERTURE_ERROR_OUT_OF_DATE:
      return "OutOfDate";
    case APERTURE_ERROR_TIMEOUT:
      return "Timeout";
    case APERTURE_ERROR_VERSION_MISMATCH:
      return "VersionMismatch";
    default:
      return "Unknown";
  }
}

#endif
