#ifndef APERTURE_MEMORY_COMMON_H
#define APERTURE_MEMORY_COMMON_H

#include <aperture/device.h>
#include <aperture/platform.h>
#include <aperture/types.h>

#include <stdint.h>

APERTURE_C_BEGIN

/// @brief Memory class of an allocation.
typedef uint8_t ApertureMemory;

enum {
  APERTURE_MEMORY_DEFAULT = 0U,
  APERTURE_MEMORY_READBACK,
};

/// @brief Name of an `ApertureMemory` enumerator.
/// @param memory Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
static inline const char* aperture_memory_to_string(ApertureMemory memory) {
  switch (memory) {
    case APERTURE_MEMORY_DEFAULT:
      return "Default";
    case APERTURE_MEMORY_READBACK:
      return "Readback";
    default:
      return "Unknown";
  }
}

/// @brief Cached device address of a host pointer from `aperture_malloc`.
/// @param device Device that owns the allocation
/// @param host Host pointer from a live `aperture_malloc`
/// @return Device address of `host`
/// @warning Asserts if `device` or `host` is null, or if `host` is unknown.
APERTURE_C_API ApertureGpuPtr aperture_device_address_of(
    ApertureDevice device, void* host) APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
