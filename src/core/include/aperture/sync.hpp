#pragma once

#include <aperture/commands.hpp>
#include <aperture/types.hpp>

#include <cstdint>
#include <string_view>

namespace aperture {

/// @brief Timeline semaphore. `Invalid` is never a live timeline.
enum class Timeline : uint8_t { Invalid = 0 };

/// @brief Name of a `Timeline` enumerator.
/// @param timeline Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(Timeline timeline) noexcept {
  switch (timeline) {
    using enum Timeline;
    case Invalid:
      return "Invalid";
  }
  return "Unknown";
}

/// @brief Split-barrier event. Requires `Capability::SplitBarriers`.
enum class Event : uint8_t { Invalid = 0 };

/// @brief Name of an `Event` enumerator.
/// @param event Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(Event event) noexcept {
  switch (event) {
    using enum Event;
    case Invalid:
      return "Invalid";
  }
  return "Unknown";
}

/// @brief Timestamp query object.
enum class TimestampQuery : uint8_t { Invalid = 0 };

/// @brief Name of a `TimestampQuery` enumerator.
/// @param query Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(
    TimestampQuery query) noexcept {
  switch (query) {
    using enum TimestampQuery;
    case Invalid:
      return "Invalid";
  }
  return "Unknown";
}

/// @brief Wait on a timeline before a set of stages may proceed.
struct TimelineWait {
  uint64_t value = 0;
  Stage wait_before = Stage::All;
  Timeline timeline = Timeline::Invalid;
};

}  // namespace aperture
