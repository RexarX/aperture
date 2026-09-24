#pragma once

#include <aperture/device.hpp>
#include <aperture/platform.hpp>
#include <aperture/result.hpp>
#include <aperture/utils/flags.hpp>

namespace aperture {

/// @brief Which work a queue family can submit.
enum class QueueUsage : uint32_t {
  Graphics = 1U << 0U,
  Compute = 1U << 1U,
  Copy = 1U << 2U,
};

/// @brief Name of a `QueueUsage` enumerator.
/// @param usage Exact enumerator. Combined masks return `"Flags"`.
/// @return Enumerator name, `"Flags"`, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(QueueUsage usage) noexcept {
  switch (usage) {
    using enum QueueUsage;
    case Graphics:
      return "Graphics";
    case Compute:
      return "Compute";
    case Copy:
      return "Copy";
  }
  return "Flags";
}

template <>
inline constexpr bool IS_FLAGS<QueueUsage> = true;

/// @brief Pointer-sized CPU handle for a queue.
struct Queue {
  void* ptr = nullptr;

  /// @brief True if this handle is non-null.
  /// @return `true` when `ptr != nullptr`
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return ptr != nullptr;
  }

  [[nodiscard]] constexpr bool operator==(const Queue&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const Queue&) const noexcept =
      default;
};

/// @brief Graphics queue. Always present on a created device.
/// @param device Device that owns the queue
/// @return Graphics queue handle
/// @warning Asserts if `device` is null.
[[nodiscard]] APERTURE_API Queue GraphicsQueue(Device device) noexcept;

/// @brief Dedicated compute queue. Requires `Capability::AsyncCompute`.
/// @param device Device that owns the queue
/// @return The queue, or `Error::Unsupported` if the device has none
/// @warning Asserts if `device` is null.
[[nodiscard]] APERTURE_API auto ComputeQueue(Device device) noexcept
    -> Result<Queue>;

/// @brief Dedicated copy queue. Requires `Capability::AsyncCopy`.
/// @param device Device that owns the queue
/// @return The queue, or `Error::Unsupported` if the device has none
/// @warning Asserts if `device` is null.
[[nodiscard]] APERTURE_API auto CopyQueue(Device device) noexcept
    -> Result<Queue>;

}  // namespace aperture
