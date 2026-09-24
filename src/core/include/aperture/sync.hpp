#pragma once

#include <aperture/commands.hpp>
#include <aperture/types.hpp>

#include <cstdint>

namespace aperture {

/// @brief Pointer-sized CPU handle for a timeline semaphore.
struct Timeline {
  void* ptr = nullptr;

  /// @brief True if this handle is non-null.
  /// @return `true` when `ptr != nullptr`
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return ptr != nullptr;
  }

  [[nodiscard]] constexpr bool operator==(const Timeline&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const Timeline&) const noexcept =
      default;
};

/// @brief Pointer-sized CPU handle for a split-barrier event.
/// @details Requires `Capability::SplitBarriers`.
struct Event {
  void* ptr = nullptr;

  /// @brief True if this handle is non-null.
  /// @return `true` when `ptr != nullptr`
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return ptr != nullptr;
  }

  [[nodiscard]] constexpr bool operator==(const Event&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const Event&) const noexcept =
      default;
};

/// @brief Wait on a timeline before a set of stages may proceed.
struct TimelineWait {
  Timeline timeline;
  uint64_t value = 0;
  Stage wait_before = Stage::All;
};

/// @brief Signal a timeline when submitted work completes.
struct TimelineSignal {
  Timeline timeline;
  uint64_t value = 0;
};

}  // namespace aperture
