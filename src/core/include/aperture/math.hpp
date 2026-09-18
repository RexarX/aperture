#pragma once

#include <aperture/assert.hpp>

#include <concepts>
#include <cstddef>
#include <cstdint>

namespace aperture {

template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template <Numeric T, size_t N>
struct vector {
  static constexpr size_t COMPONENT_COUNT = N;

  T v[N] = {};

  [[nodiscard]] constexpr T& operator[](size_t index) noexcept {
    APERTURE_ASSERT(index < N);
    return v[index];
  }

  [[nodiscard]] constexpr const T& operator[](size_t index) const noexcept {
    APERTURE_ASSERT(index < N);
    return v[index];
  }

  [[nodiscard]] constexpr bool operator==(const vector&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const vector&) const noexcept =
      default;
};

template <Numeric T, size_t R, size_t C>
  requires((R * C) > 1)
struct matrix {
  static constexpr size_t ROW_COUNT = R;
  static constexpr size_t COLUMN_COUNT = C;

  T m[R * C] = {};

  struct ColumnProxy {
    T* col = nullptr;

    [[nodiscard]] constexpr T& operator[](size_t row) noexcept {
      APERTURE_ASSERT(row < R);
      return col[row];
    }

    [[nodiscard]] constexpr const T& operator[](size_t row) const noexcept {
      APERTURE_ASSERT(row < R);
      return col[row];
    }
  };

  struct ConstColumnProxy {
    const T* col = nullptr;

    [[nodiscard]] constexpr const T& operator[](size_t row) const noexcept {
      APERTURE_ASSERT(row < R);
      return col[row];
    }
  };

  [[nodiscard]] constexpr ColumnProxy operator[](size_t col) noexcept {
    APERTURE_ASSERT(col < C);
    return {m + (col * R)};
  }

  [[nodiscard]] constexpr ConstColumnProxy operator[](
      size_t col) const noexcept {
    APERTURE_ASSERT(col < C);
    return {m + (col * R)};
  }

  [[nodiscard]] constexpr bool operator==(const matrix&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const matrix&) const noexcept =
      default;
};

using int2 = vector<int32_t, 2>;
using int3 = vector<int32_t, 3>;
using int4 = vector<int32_t, 4>;

using uint2 = vector<uint32_t, 2>;
using uint3 = vector<uint32_t, 3>;
using uint4 = vector<uint32_t, 4>;

using float2 = vector<float, 2>;
using float3 = vector<float, 3>;
using float4 = vector<float, 4>;

using int2x2 = matrix<int32_t, 2, 2>;
using int3x3 = matrix<int32_t, 3, 3>;
using int3x4 = matrix<int32_t, 3, 4>;
using int4x3 = matrix<int32_t, 4, 3>;
using int4x4 = matrix<int32_t, 4, 4>;

using uint2x2 = matrix<uint32_t, 2, 2>;
using uint3x3 = matrix<uint32_t, 3, 3>;
using uint3x4 = matrix<uint32_t, 3, 4>;
using uint4x3 = matrix<uint32_t, 4, 3>;
using uint4x4 = matrix<uint32_t, 4, 4>;

using float2x2 = matrix<float, 2, 2>;
using float3x3 = matrix<float, 3, 3>;
using float3x4 = matrix<float, 3, 4>;
using float4x3 = matrix<float, 4, 3>;
using float4x4 = matrix<float, 4, 4>;

}  // namespace aperture
