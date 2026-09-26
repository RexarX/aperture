#pragma once

#include <aperture/command/buffer.hpp>
#include <aperture/device.hpp>
#include <aperture/platform.hpp>
#include <aperture/queue.hpp>
#include <aperture/result.hpp>
#include <aperture/types.hpp>

#include <cstdint>
#include <span>

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

/// @brief One queue submission.
/// @details `buffers` are consumed: each token is empty after a successful
/// submit. Empty `waits` is no GPU wait. Empty `signals` is no signal.
struct SubmitDesc {
  std::span<CommandBuffer> buffers;
  std::span<const TimelineWait> waits;
  std::span<const TimelineSignal> signals;
};

/// @brief Creates a timeline semaphore.
/// @param device Device that owns the semaphore
/// @param initial Starting counter value
/// @return The timeline, or a recoverable `Error`
/// @warning Asserts if `device` is null.
[[nodiscard]] APERTURE_API auto CreateTimeline(Device device,
                                               uint64_t initial = 0) noexcept
    -> Result<Timeline>;

/// @brief Blocks until `timeline` reaches `value`.
/// @param timeline Timeline to wait on
/// @param value Counter value to wait for
/// @warning Asserts in next cases:
/// - If `timeline` is null
/// - If the device is lost (`APERTURE_VERIFY`)
/// @note Does not time out. Safe to call from any thread.
APERTURE_API void Wait(Timeline timeline, uint64_t value) noexcept;

/// @brief Waits until `timeline` reaches `value`, or `timeout_ns` elapses.
/// @param timeline Timeline to wait on
/// @param value Counter value to wait for
/// @param timeout_ns Maximum wait in nanoseconds
/// @return Nothing, or `Error::Timeout` / `Error::DeviceLost`
/// @warning Asserts if `timeline` is null.
/// @warning Safe to call from any thread.
[[nodiscard]] APERTURE_API auto Wait(Timeline timeline, uint64_t value,
                                     uint64_t timeout_ns) noexcept
    -> Result<void>;

/// @brief Latest signaled value of `timeline`. Does not block.
/// @param timeline Timeline to read
/// @return The current counter
/// @warning Asserts if `timeline` is null.
/// @warning Safe to call from any thread.
[[nodiscard]] APERTURE_API uint64_t CurrentValue(Timeline timeline) noexcept;

/// @brief Destroys a timeline. Immediate; does not wait on the GPU.
/// @param timeline Timeline to destroy. Null is a no-op
APERTURE_API void Destroy(Timeline timeline) noexcept;

/// @brief Submits `desc.buffers` to `queue`.
/// @param queue Queue that runs the buffers
/// @param desc Buffers, GPU waits, and signals
/// @return Nothing, or `Error::Invalid` / `Error::DeviceLost` /
/// `Error::OutOfMemory`
/// @warning Asserts in next cases:
/// - If `queue` is null
/// - If a command buffer is null
/// - If a command buffer is not recording
/// @warning An empty buffer span is `Error::Invalid`. A buffer whose pool
/// queue is not `queue` is `Error::Invalid`.
/// @warning `Submit` on one queue is externally synchronized.
[[nodiscard]] APERTURE_API auto Submit(Queue queue,
                                       const SubmitDesc& desc) noexcept
    -> Result<void>;

}  // namespace aperture
