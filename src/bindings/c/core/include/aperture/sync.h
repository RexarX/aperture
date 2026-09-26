#ifndef APERTURE_SYNC_H
#define APERTURE_SYNC_H

#include <aperture/commands.h>
#include <aperture/device.h>
#include <aperture/platform.h>
#include <aperture/queue.h>
#include <aperture/result.h>

#include <stddef.h>
#include <stdint.h>

APERTURE_C_BEGIN

/// @brief Pointer-sized CPU handle for a timeline semaphore. Invalid is `NULL`.
typedef struct ApertureTimelineImpl* ApertureTimeline;

/// @brief Split-barrier event. Requires `APERTURE_CAPABILITY_SPLIT_BARRIERS`.
/// Invalid is `NULL`.
typedef struct ApertureEventImpl* ApertureEvent;

/// @brief Wait on a timeline before a set of stages may proceed.
typedef struct ApertureTimelineWait {
  ApertureTimeline timeline;
  uint64_t value;
  ApertureStage wait_before;
} ApertureTimelineWait;

/// @brief Signal a timeline when submitted work completes.
typedef struct ApertureTimelineSignal {
  ApertureTimeline timeline;
  uint64_t value;
} ApertureTimelineSignal;

/// @brief One queue submission. `buffers` are consumed: each slot is set to
/// `NULL` when that token was submitted.
typedef struct ApertureSubmitDesc {
  ApertureCommandBuffer* buffers;
  size_t buffer_count;
  const ApertureTimelineWait* waits;
  size_t wait_count;
  const ApertureTimelineSignal* signals;
  size_t signal_count;
} ApertureSubmitDesc;

/// @brief Creates a timeline semaphore.
/// @param device Device that owns the semaphore
/// @param initial Starting counter value
/// @param out Receives the timeline on success
/// @return The creation error, or `APERTURE_ERROR_OK`
/// @warning Asserts in next cases:
/// - If `device` is null
/// - If `out` is null
APERTURE_C_API ApertureError
aperture_create_timeline(ApertureDevice device, uint64_t initial,
                         ApertureTimeline* out) APERTURE_C_NOEXCEPT;

/// @brief Blocks until `timeline` reaches `value`.
/// @param timeline Timeline to wait on
/// @param value Counter value to wait for
/// @warning Asserts in next cases:
/// - If `timeline` is null
/// - If the device is lost
APERTURE_C_API void aperture_wait_timeline(ApertureTimeline timeline,
                                           uint64_t value) APERTURE_C_NOEXCEPT;

/// @brief Waits until `timeline` reaches `value`, or `timeout_ns` elapses.
/// @param timeline Timeline to wait on
/// @param value Counter value to wait for
/// @param timeout_ns Maximum wait in nanoseconds
/// @return The wait error, or `APERTURE_ERROR_OK`
/// @warning Asserts if `timeline` is null.
APERTURE_C_API ApertureError
aperture_wait_timeline_timeout(ApertureTimeline timeline, uint64_t value,
                               uint64_t timeout_ns) APERTURE_C_NOEXCEPT;

/// @brief Latest signaled value of `timeline`. Does not block.
/// @param timeline Timeline to read
/// @return The current counter
/// @warning Asserts if `timeline` is null.
APERTURE_C_API uint64_t
aperture_timeline_current_value(ApertureTimeline timeline) APERTURE_C_NOEXCEPT;

/// @brief Destroys a timeline. Does not wait on the GPU.
/// @param timeline Timeline to destroy. Null is a no-op
APERTURE_C_API void aperture_destroy_timeline(ApertureTimeline timeline)
    APERTURE_C_NOEXCEPT;

/// @brief Submits `desc->buffers` to `queue`.
/// @param queue Queue that runs the buffers
/// @param desc Buffers, GPU waits, and signals
/// @return The submit error, or `APERTURE_ERROR_OK`
/// @warning Asserts in next cases:
/// - If `queue` is null
/// - If `desc` is null
/// - If a command buffer is null
/// - If a command buffer is not recording
/// @warning Consumed buffer slots are set to `NULL`.
APERTURE_C_API ApertureError aperture_submit(
    ApertureQueue queue, const ApertureSubmitDesc* desc) APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
