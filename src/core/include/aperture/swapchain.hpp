#pragma once

#include <cstdint>
#include <string_view>
#include <utility>

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

[[nodiscard]] constexpr PresentMode operator|(PresentMode lhs,
                                              PresentMode rhs) noexcept {
  return static_cast<PresentMode>(std::to_underlying(lhs) |
                                  std::to_underlying(rhs));
}

[[nodiscard]] constexpr PresentMode operator&(PresentMode lhs,
                                              PresentMode rhs) noexcept {
  return static_cast<PresentMode>(std::to_underlying(lhs) &
                                  std::to_underlying(rhs));
}

[[nodiscard]] constexpr PresentMode operator~(PresentMode value) noexcept {
  return static_cast<PresentMode>(~std::to_underlying(value));
}

constexpr PresentMode& operator|=(PresentMode& lhs, PresentMode rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr PresentMode& operator&=(PresentMode& lhs, PresentMode rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

/// @brief True if every bit in `bits` is set in `set`.
/// @param set Mask to test
/// @param bits Required bits
/// @return `true` if `(set & bits) == bits`
[[nodiscard]] constexpr bool HasAll(PresentMode set,
                                    PresentMode bits) noexcept {
  return (set & bits) == bits;
}

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
