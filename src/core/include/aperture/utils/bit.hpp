#pragma once

#include <bit>
#include <concepts>
#include <limits>

namespace aperture::utils {

/// @brief True if `value` is a non-zero power of two.
/// @tparam T Unsigned integer type
/// @param value Value to test
/// @return `true` if `std::has_single_bit(value)`
template <std::unsigned_integral T>
[[nodiscard]] constexpr bool IsPowerOfTwo(T value) noexcept {
  return std::has_single_bit(value);
}

/// @brief Rounds `value` up to the next multiple of `align`.
/// @tparam T Unsigned integer type
/// @param value Value to align
/// @param align Alignment. Must be a non-zero power of two
/// @return Smallest multiple of `align` that is >= `value`
/// @warning Behavior is undefined if `align` is not a non-zero power of two.
/// @warning Wraps on overflow of `value + (align - 1)`.
template <std::unsigned_integral T>
[[nodiscard]] constexpr T AlignUp(T value, T align) noexcept {
  return (value + (align - 1)) & ~(align - 1);
}

/// @brief Rounds `value` down to the previous multiple of `align`.
/// @tparam T Unsigned integer type
/// @param value Value to align
/// @param align Alignment. Must be a non-zero power of two
/// @return Largest multiple of `align` that is <= `value`
/// @warning Behavior is undefined if `align` is not a non-zero power of two.
template <std::unsigned_integral T>
[[nodiscard]] constexpr T AlignDown(T value, T align) noexcept {
  return value & ~(align - 1);
}

/// @brief True if `a * b` would overflow `T`.
/// @tparam T Unsigned integer type
/// @param a Left factor
/// @param b Right factor
/// @return `true` if the product does not fit in `T`
template <std::unsigned_integral T>
[[nodiscard]] constexpr bool MulOverflows(T lhs, T rhs) noexcept {
  return rhs != 0 && lhs > (std::numeric_limits<T>::max() / rhs);
}

}  // namespace aperture::utils
