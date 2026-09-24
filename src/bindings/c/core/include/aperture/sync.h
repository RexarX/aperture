#ifndef APERTURE_SYNC_H
#define APERTURE_SYNC_H

#include <aperture/commands.h>
#include <stdint.h>

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

#endif
