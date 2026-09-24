#ifndef APERTURE_SYNC_H
#define APERTURE_SYNC_H

#include <aperture/commands.h>
#include <stdint.h>

/// @brief Timeline semaphore. `INVALID` is never a live timeline.
typedef struct ApertureTimeline {
  uint8_t id;
} ApertureTimeline;

enum { APERTURE_TIMELINE_INVALID = 0U };

/// @brief Split-barrier event. Requires `APERTURE_CAPABILITY_SPLIT_BARRIERS`.
typedef struct ApertureEvent {
  uint8_t id;
} ApertureEvent;

enum { APERTURE_EVENT_INVALID = 0U };

/// @brief Wait on a timeline before a set of stages may proceed.
typedef struct ApertureTimelineWait {
  uint64_t value;
  ApertureStage wait_before;
  ApertureTimeline timeline;
} ApertureTimelineWait;

#endif
