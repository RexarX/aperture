#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

namespace aperture {

/// @brief True when `E` is a bitmask enum with flag operators.
/// @details Specialize to `true` after the enum definition. Closed enumerators
/// stay `false` and do not gain bitwise operators.
/// @tparam E Scoped enum type
template <typename E>
inline constexpr bool IS_FLAGS = false;

/// @brief Scoped enum opted into flag operators via `IS_FLAGS`.
/// @tparam E Scoped enum type
template <typename E>
concept Flags = std::is_scoped_enum_v<E> && IS_FLAGS<E>;

/// @brief Bitwise or of two flag values.
/// @tparam E Flag enum
/// @param lhs Left mask
/// @param rhs Right mask
/// @return Union of the bits
template <Flags E>
[[nodiscard]] constexpr E operator|(E lhs, E rhs) noexcept {
  return static_cast<E>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

/// @brief Bitwise and of two flag values.
/// @tparam E Flag enum
/// @param lhs Left mask
/// @param rhs Right mask
/// @return Intersection of the bits
template <Flags E>
[[nodiscard]] constexpr E operator&(E lhs, E rhs) noexcept {
  return static_cast<E>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

/// @brief Bitwise complement of a flag value.
/// @tparam E Flag enum
/// @param value Mask to invert
/// @return Inverted bits, truncated to `E`'s underlying type
template <Flags E>
[[nodiscard]] constexpr E operator~(E value) noexcept {
  return static_cast<E>(~std::to_underlying(value));
}

/// @brief Bitwise or-assign.
/// @tparam E Flag enum
/// @param lhs Mask to update
/// @param rhs Bits to set
/// @return `lhs` after the update
template <Flags E>
constexpr E& operator|=(E& lhs, E rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

/// @brief Bitwise and-assign.
/// @tparam E Flag enum
/// @param lhs Mask to update
/// @param rhs Bits to keep
/// @return `lhs` after the update
template <Flags E>
constexpr E& operator&=(E& lhs, E rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

/// @brief True if every bit in `bits` is set in `set`.
/// @tparam E Flag enum
/// @param set Mask to test
/// @param bits Required bits
/// @return `true` if `(set & bits) == bits`
template <Flags E>
[[nodiscard]] constexpr bool HasAll(E set, E bits) noexcept {
  return (set & bits) == bits;
}

}  // namespace aperture
