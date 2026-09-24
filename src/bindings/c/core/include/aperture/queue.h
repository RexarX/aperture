#ifndef APERTURE_QUEUE_H
#define APERTURE_QUEUE_H

#include <aperture/device.h>
#include <aperture/platform.h>
#include <aperture/result.h>

#include <stdbool.h>
#include <stdint.h>

APERTURE_C_BEGIN

/// @brief Which work a queue family can submit.
typedef uint32_t ApertureQueueUsage;

enum {
  APERTURE_QUEUE_USAGE_GRAPHICS = 1U << 0U,
  APERTURE_QUEUE_USAGE_COMPUTE = 1U << 1U,
  APERTURE_QUEUE_USAGE_COPY = 1U << 2U,
};

/// @brief Name of an `ApertureQueueUsage` enumerator.
/// @param usage Exact enumerator. Combined masks return `"Flags"`.
/// @return Enumerator name, or `"Flags"`
static inline const char* aperture_queue_usage_to_string(
    ApertureQueueUsage usage) {
  switch (usage) {
    case APERTURE_QUEUE_USAGE_GRAPHICS:
      return "Graphics";
    case APERTURE_QUEUE_USAGE_COMPUTE:
      return "Compute";
    case APERTURE_QUEUE_USAGE_COPY:
      return "Copy";
    default:
      return "Flags";
  }
}

/// @brief True if every bit in `bits` is set in `set`.
/// @param set Mask to test
/// @param bits Required bits
/// @return `true` if `(set & bits) == bits`
static inline bool aperture_queue_usage_has_all(ApertureQueueUsage set,
                                                ApertureQueueUsage bits) {
  return (ApertureQueueUsage)(set & bits) == bits;
}

/// @brief Pointer-sized CPU handle for a queue. Invalid is `NULL`.
typedef struct ApertureQueueImpl* ApertureQueue;

/// @brief Graphics queue. Always present on a created device.
/// @param device Device that owns the queue
/// @return Graphics queue handle
/// @warning Asserts if `device` is null.
APERTURE_C_API ApertureQueue aperture_graphics_queue(ApertureDevice device)
    APERTURE_C_NOEXCEPT;

/// @brief Dedicated compute queue. Requires
/// `APERTURE_CAPABILITY_ASYNC_COMPUTE`.
/// @param device Device that owns the queue
/// @param out Receives the queue on success
/// @return `APERTURE_ERROR_OK`, or `APERTURE_ERROR_UNSUPPORTED` if none
/// @warning Asserts if `device` or `out` is null.
APERTURE_C_API ApertureError aperture_compute_queue(
    ApertureDevice device, ApertureQueue* out) APERTURE_C_NOEXCEPT;

/// @brief Dedicated copy queue. Requires `APERTURE_CAPABILITY_ASYNC_COPY`.
/// @param device Device that owns the queue
/// @param out Receives the queue on success
/// @return `APERTURE_ERROR_OK`, or `APERTURE_ERROR_UNSUPPORTED` if none
/// @warning Asserts if `device` or `out` is null.
APERTURE_C_API ApertureError aperture_copy_queue(
    ApertureDevice device, ApertureQueue* out) APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
