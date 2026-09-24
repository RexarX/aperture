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

  T data[N] = {};

  [[nodiscard]] constexpr T& operator[](size_t index) noexcept {
    APERTURE_ASSERT(index < N);
    return data[index];
  }

  [[nodiscard]] constexpr const T& operator[](size_t index) const noexcept {
    APERTURE_ASSERT(index < N);
    return data[index];
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

  T data[R * C] = {};

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
    return {data + (col * R)};
  }

  [[nodiscard]] constexpr ConstColumnProxy operator[](
      size_t col) const noexcept {
    APERTURE_ASSERT(col < C);
    return {data + (col * R)};
  }

  [[nodiscard]] constexpr T& operator[](size_t row, size_t col) noexcept {
    APERTURE_ASSERT(row < R);
    APERTURE_ASSERT(col < C);
    return data[(col * R) + row];
  }

  [[nodiscard]] constexpr const T& operator[](size_t row,
                                              size_t col) const noexcept {
    APERTURE_ASSERT(row < R);
    APERTURE_ASSERT(col < C);
    return data[(col * R) + row];
  }

  constexpr void SetColumn(size_t col, const vector<T, R>& vec) noexcept {
    APERTURE_ASSERT(col < C);
    for (size_t row = 0; row < R; ++row) {
      data[(col * R) + row] = vec[row];
    }
  }

  constexpr void SetRow(size_t row, const vector<T, C>& vec) noexcept {
    APERTURE_ASSERT(row < R);
    for (size_t col = 0; col < C; ++col) {
      data[(col * R) + row] = vec[col];
    }
  }

  [[nodiscard]] constexpr auto GetColumn(size_t col) const noexcept
      -> vector<T, R> {
    APERTURE_ASSERT(col < C);
    vector<T, R> result;
    for (size_t row = 0; row < R; ++row) {
      result[row] = data[(col * R) + row];
    }
    return result;
  }

  [[nodiscard]] constexpr auto GetRow(size_t row) const noexcept
      -> vector<T, C> {
    APERTURE_ASSERT(row < R);
    vector<T, C> result;
    for (size_t col = 0; col < C; ++col) {
      result[col] = data[(col * R) + row];
    }
    return result;
  }

  [[nodiscard]] constexpr bool operator==(const matrix&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const matrix&) const noexcept =
      default;
};

#define APERTURE_VECTOR_ALIASES(T, name) \
  using name##2 = vector<T, 2>;          \
  using name##3 = vector<T, 3>;          \
  using name##4 = vector<T, 4>;

APERTURE_VECTOR_ALIASES(int32_t, int)
APERTURE_VECTOR_ALIASES(uint32_t, uint)
APERTURE_VECTOR_ALIASES(float, float)

#undef APERTURE_VECTOR_ALIASES

#define APERTURE_MATRIX_ALIASES(T, name) \
  using name##2x2 = matrix<T, 2, 2>;     \
  using name##2x3 = matrix<T, 2, 3>;     \
  using name##2x4 = matrix<T, 2, 4>;     \
  using name##3x2 = matrix<T, 3, 2>;     \
  using name##3x3 = matrix<T, 3, 3>;     \
  using name##3x4 = matrix<T, 3, 4>;     \
  using name##4x2 = matrix<T, 4, 2>;     \
  using name##4x3 = matrix<T, 4, 3>;     \
  using name##4x4 = matrix<T, 4, 4>;

APERTURE_MATRIX_ALIASES(int32_t, int)
APERTURE_MATRIX_ALIASES(uint32_t, uint)
APERTURE_MATRIX_ALIASES(float, float)

#undef APERTURE_MATRIX_ALIASES

}  // namespace aperture
