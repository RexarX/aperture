#ifndef APERTURE_MEMORY_MALLOC_H
#define APERTURE_MEMORY_MALLOC_H

#include <aperture/device.h>
#include <aperture/memory/common.h>
#include <aperture/platform.h>
#include <aperture/queue.h>
#include <aperture/result.h>
#include <aperture/types.h>

#include <stddef.h>
#include <stdint.h>

APERTURE_C_BEGIN

/// @brief Placement flags for malloc.
typedef uint32_t ApertureMallocFlags;

enum {
  APERTURE_MALLOC_FLAGS_NONE = 0U,
  APERTURE_MALLOC_FLAGS_DEDICATED = 1U << 0U,
};

/// @brief Name of an `ApertureMallocFlags` enumerator.
/// @param flags Exact enumerator or `NONE`. Combined masks return `"Flags"`.
/// @return Enumerator name, `"Flags"`, or `"Unknown"`
static inline const char* aperture_malloc_flags_to_string(
    ApertureMallocFlags flags) {
  switch (flags) {
    case APERTURE_MALLOC_FLAGS_NONE:
      return "None";
    case APERTURE_MALLOC_FLAGS_DEDICATED:
      return "Dedicated";
    default:
      return "Flags";
  }
}

/// @brief Allocates host-mapped device-local memory.
/// @param device Device that owns the allocation
/// @param bytes Size in bytes
/// @param align Alignment in bytes. Must be a non-zero power of two
/// @param memory `DEFAULT` or `READBACK`
/// @param usage Queues that may touch this allocation
/// @param flags `DEDICATED` returns an exclusive block
/// @param out Receives host pointer plus device address on success
/// @return The allocation error, or `APERTURE_ERROR_OK`
/// @warning Asserts if `device` or `out` is null.
APERTURE_C_API ApertureError aperture_malloc(
    ApertureDevice device, size_t bytes, size_t align, ApertureMemory memory,
    ApertureQueueUsage usage, ApertureMallocFlags flags,
    ApertureDualPtr* out) APERTURE_C_NOEXCEPT;

/// @brief Allocates device-only memory. Not CPU-writable.
/// @param device Device that owns the allocation
/// @param bytes Size in bytes
/// @param align Alignment in bytes. Must be a non-zero power of two
/// @param usage Queues that may touch this allocation
/// @param flags `DEDICATED` returns an exclusive block
/// @param out Receives the device address on success
/// @return The allocation error, or `APERTURE_ERROR_OK`
/// @warning Asserts if `device` or `out` is null.
APERTURE_C_API ApertureError aperture_malloc_gpu(
    ApertureDevice device, size_t bytes, size_t align, ApertureQueueUsage usage,
    ApertureMallocFlags flags, ApertureGpuPtr* out) APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
