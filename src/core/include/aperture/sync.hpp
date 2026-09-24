#pragma once

#include <aperture/commands.hpp>
#include <aperture/types.hpp>

#include <cstdint>

namespace aperture {

/// @brief Timeline semaphore. `Invalid` is never a live timeline.
enum class Timeline : uint8_t { Invalid = 0 };

/// @brief Split-barrier event. Requires `Capability::SplitBarriers`.
enum class Event : uint8_t { Invalid = 0 };

/// @brief Wait on a timeline before a set of stages may proceed.
struct TimelineWait {
  uint64_t value = 0;
  Stage wait_before = Stage::All;
  Timeline timeline = Timeline::Invalid;
};

}  // namespace aperture
