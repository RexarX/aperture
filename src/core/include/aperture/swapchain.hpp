#pragma once

#include <aperture/utils/flags.hpp>

#include <cstdint>
#include <string_view>

namespace aperture {

/// @brief Presentation modes reported by an adapter, as a bit mask.
enum class PresentMode : uint16_t {
  None = 0,
  Fifo = 1U << 0U,
  Immediate = 1U << 1U,
  Mailbox = 1U << 2U,
  Waitable = 1U << 3U,
};

/// @brief Name of a `PresentMode` enumerator.
/// @param mode Exact enumerator or `None`. Combined masks return `"Flags"`.
/// @return Enumerator name, `"Flags"`, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(PresentMode mode) noexcept {
  switch (mode) {
    using enum PresentMode;
    case None:
      return "None";
    case Fifo:
      return "Fifo";
    case Immediate:
      return "Immediate";
    case Mailbox:
      return "Mailbox";
    case Waitable:
      return "Waitable";
  }
  return "Flags";
}

template <>
inline constexpr bool IS_FLAGS<PresentMode> = true;

/// @brief Swapchain. Pointer-sized CPU handle.
struct Swapchain {
  void* ptr = nullptr;

  /// @brief True if this handle is non-null.
  /// @return `true` when `ptr != nullptr`
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return ptr != nullptr;
  }
  [[nodiscard]] constexpr bool operator==(const Swapchain&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(
      const Swapchain& other) const noexcept = default;
};

}  // namespace aperture
