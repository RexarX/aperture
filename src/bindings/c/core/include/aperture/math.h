#ifndef APERTURE_MATH_H
#define APERTURE_MATH_H

#include <aperture/assert.h>

#include <stddef.h>
#include <stdint.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

/// @file
/// @brief C mirrors of `aperture::vector` / `aperture::matrix`.
/// @details Vectors are anonymous-struct unions so `v.x` and `v.data[i]`
/// alias. Matrices are column-major: `data[(col * rows) + row]`, with
/// `columns[col]` as the column vector. Helpers assert on null and
/// out-of-range indices.

/// @brief 2-component signed integer vector.
typedef union ApertureInt2 {
  int32_t data[2];
  struct {
    int32_t x, y;
  };
} ApertureInt2;

/// @brief 2-component unsigned integer vector.
typedef union ApertureUint2 {
  uint32_t data[2];
  struct {
    uint32_t x, y;
  };
} ApertureUint2;

/// @brief 2-component float vector.
typedef union ApertureFloat2 {
  float data[2];
  struct {
    float x, y;
  };
} ApertureFloat2;

/// @brief 3-component signed integer vector.
typedef union ApertureInt3 {
  int32_t data[3];
  struct {
    int32_t x, y, z;
  };
} ApertureInt3;

/// @brief 3-component unsigned integer vector.
typedef union ApertureUint3 {
  uint32_t data[3];
  struct {
    uint32_t x, y, z;
  };
} ApertureUint3;

/// @brief 3-component float vector.
typedef union ApertureFloat3 {
  float data[3];
  struct {
    float x, y, z;
  };
  struct {
    float r, g, b;
  };
} ApertureFloat3;

/// @brief 4-component signed integer vector.
typedef union ApertureInt4 {
  int32_t data[4];
  struct {
    int32_t x, y, z, w;
  };
} ApertureInt4;

/// @brief 4-component unsigned integer vector.
typedef union ApertureUint4 {
  uint32_t data[4];
  struct {
    uint32_t x, y, z, w;
  };
} ApertureUint4;

/// @brief 4-component float vector.
typedef union ApertureFloat4 {
  float data[4];
  struct {
    float x, y, z, w;
  };
  struct {
    float r, g, b, a;
  };
} ApertureFloat4;

/// @brief Column-major 2x2 signed integer matrix.
typedef union ApertureInt2x2 {
  int32_t data[4];
  ApertureInt2 columns[2];
} ApertureInt2x2;

/// @brief Column-major 2x3 signed integer matrix.
typedef union ApertureInt2x3 {
  int32_t data[6];
  ApertureInt2 columns[3];
} ApertureInt2x3;

/// @brief Column-major 2x4 signed integer matrix.
typedef union ApertureInt2x4 {
  int32_t data[8];
  ApertureInt2 columns[4];
} ApertureInt2x4;

/// @brief Column-major 3x2 signed integer matrix.
typedef union ApertureInt3x2 {
  int32_t data[6];
  ApertureInt3 columns[2];
} ApertureInt3x2;

/// @brief Column-major 3x3 signed integer matrix.
typedef union ApertureInt3x3 {
  int32_t data[9];
  ApertureInt3 columns[3];
} ApertureInt3x3;

/// @brief Column-major 3x4 signed integer matrix.
typedef union ApertureInt3x4 {
  int32_t data[12];
  ApertureInt3 columns[4];
} ApertureInt3x4;

/// @brief Column-major 4x2 signed integer matrix.
typedef union ApertureInt4x2 {
  int32_t data[8];
  ApertureInt4 columns[2];
} ApertureInt4x2;

/// @brief Column-major 4x3 signed integer matrix.
typedef union ApertureInt4x3 {
  int32_t data[12];
  ApertureInt4 columns[3];
} ApertureInt4x3;

/// @brief Column-major 4x4 signed integer matrix.
typedef union ApertureInt4x4 {
  int32_t data[16];
  ApertureInt4 columns[4];
} ApertureInt4x4;

/// @brief Column-major 2x2 unsigned integer matrix.
typedef union ApertureUint2x2 {
  uint32_t data[4];
  ApertureUint2 columns[2];
} ApertureUint2x2;

/// @brief Column-major 2x3 unsigned integer matrix.
typedef union ApertureUint2x3 {
  uint32_t data[6];
  ApertureUint2 columns[3];
} ApertureUint2x3;

/// @brief Column-major 2x4 unsigned integer matrix.
typedef union ApertureUint2x4 {
  uint32_t data[8];
  ApertureUint2 columns[4];
} ApertureUint2x4;

/// @brief Column-major 3x2 unsigned integer matrix.
typedef union ApertureUint3x2 {
  uint32_t data[6];
  ApertureUint3 columns[2];
} ApertureUint3x2;

/// @brief Column-major 3x3 unsigned integer matrix.
typedef union ApertureUint3x3 {
  uint32_t data[9];
  ApertureUint3 columns[3];
} ApertureUint3x3;

/// @brief Column-major 3x4 unsigned integer matrix.
typedef union ApertureUint3x4 {
  uint32_t data[12];
  ApertureUint3 columns[4];
} ApertureUint3x4;

/// @brief Column-major 4x2 unsigned integer matrix.
typedef union ApertureUint4x2 {
  uint32_t data[8];
  ApertureUint4 columns[2];
} ApertureUint4x2;

/// @brief Column-major 4x3 unsigned integer matrix.
typedef union ApertureUint4x3 {
  uint32_t data[12];
  ApertureUint4 columns[3];
} ApertureUint4x3;

/// @brief Column-major 4x4 unsigned integer matrix.
typedef union ApertureUint4x4 {
  uint32_t data[16];
  ApertureUint4 columns[4];
} ApertureUint4x4;

/// @brief Column-major 2x2 float matrix.
typedef union ApertureFloat2x2 {
  float data[4];
  ApertureFloat2 columns[2];
} ApertureFloat2x2;

/// @brief Column-major 2x3 float matrix.
typedef union ApertureFloat2x3 {
  float data[6];
  ApertureFloat2 columns[3];
} ApertureFloat2x3;

/// @brief Column-major 2x4 float matrix.
typedef union ApertureFloat2x4 {
  float data[8];
  ApertureFloat2 columns[4];
} ApertureFloat2x4;

/// @brief Column-major 3x2 float matrix.
typedef union ApertureFloat3x2 {
  float data[6];
  ApertureFloat3 columns[2];
} ApertureFloat3x2;

/// @brief Column-major 3x3 float matrix.
typedef union ApertureFloat3x3 {
  float data[9];
  ApertureFloat3 columns[3];
} ApertureFloat3x3;

/// @brief Column-major 3x4 float matrix.
typedef union ApertureFloat3x4 {
  float data[12];
  ApertureFloat3 columns[4];
} ApertureFloat3x4;

/// @brief Column-major 4x2 float matrix.
typedef union ApertureFloat4x2 {
  float data[8];
  ApertureFloat4 columns[2];
} ApertureFloat4x2;

/// @brief Column-major 4x3 float matrix.
typedef union ApertureFloat4x3 {
  float data[12];
  ApertureFloat4 columns[3];
} ApertureFloat4x3;

/// @brief Column-major 4x4 float matrix.
typedef union ApertureFloat4x4 {
  float data[16];
  ApertureFloat4 columns[4];
} ApertureFloat4x4;

// clang-format off

#define APERTURE_MATH_VEC2(fn, Type, T)                                       \
  static inline Type aperture_##fn(T x, T y) {                                \
    Type v;                                                                   \
    v.x = x;                                                                  \
    v.y = y;                                                                  \
    return v;                                                                 \
  }                                                                           \
  static inline T aperture_##fn##_get(const Type* v, size_t index) {          \
    APERTURE_C_ASSERT(v != NULL);                                             \
    APERTURE_C_ASSERT(index < 2U);                                            \
    return v->data[index];                                                    \
  }                                                                           \
  static inline void aperture_##fn##_set(Type* v, size_t index, T value) {    \
    APERTURE_C_ASSERT(v != NULL);                                             \
    APERTURE_C_ASSERT(index < 2U);                                            \
    v->data[index] = value;                                                   \
  }

#define APERTURE_MATH_VEC3(fn, Type, T)                                       \
  static inline Type aperture_##fn(T x, T y, T z) {                           \
    Type v;                                                                   \
    v.x = x;                                                                  \
    v.y = y;                                                                  \
    v.z = z;                                                                  \
    return v;                                                                 \
  }                                                                           \
  static inline T aperture_##fn##_get(const Type* v, size_t index) {          \
    APERTURE_C_ASSERT(v != NULL);                                             \
    APERTURE_C_ASSERT(index < 3U);                                            \
    return v->data[index];                                                    \
  }                                                                           \
  static inline void aperture_##fn##_set(Type* v, size_t index, T value) {    \
    APERTURE_C_ASSERT(v != NULL);                                             \
    APERTURE_C_ASSERT(index < 3U);                                            \
    v->data[index] = value;                                                   \
  }

#define APERTURE_MATH_VEC4(fn, Type, T)                                       \
  static inline Type aperture_##fn(T x, T y, T z, T w) {                      \
    Type v;                                                                   \
    v.x = x;                                                                  \
    v.y = y;                                                                  \
    v.z = z;                                                                  \
    v.w = w;                                                                  \
    return v;                                                                 \
  }                                                                           \
  static inline T aperture_##fn##_get(const Type* v, size_t index) {          \
    APERTURE_C_ASSERT(v != NULL);                                             \
    APERTURE_C_ASSERT(index < 4U);                                            \
    return v->data[index];                                                    \
  }                                                                           \
  static inline void aperture_##fn##_set(Type* v, size_t index, T value) {    \
    APERTURE_C_ASSERT(v != NULL);                                             \
    APERTURE_C_ASSERT(index < 4U);                                            \
    v->data[index] = value;                                                   \
  }

/// @brief Constructs a 2-component vector from `x`, `y`.
/// @param x First component
/// @param y Second component
/// @return Vector with those components
///
/// @brief Indexed vector component.
/// @param v Vector; must not be `NULL`
/// @param index Component index in `[0, N)`
/// @return Component at `index`
/// @warning Asserts if `v` is null or `index` is out of range.
///
/// @brief Writes a vector component.
/// @param v Vector; must not be `NULL`
/// @param index Component index in `[0, N)`
/// @param value Component to store
/// @warning Asserts if `v` is null or `index` is out of range.
APERTURE_MATH_VEC2(int2, ApertureInt2, int32_t)
APERTURE_MATH_VEC2(uint2, ApertureUint2, uint32_t)
APERTURE_MATH_VEC2(float2, ApertureFloat2, float)
APERTURE_MATH_VEC3(int3, ApertureInt3, int32_t)
APERTURE_MATH_VEC3(uint3, ApertureUint3, uint32_t)
APERTURE_MATH_VEC3(float3, ApertureFloat3, float)
APERTURE_MATH_VEC4(int4, ApertureInt4, int32_t)
APERTURE_MATH_VEC4(uint4, ApertureUint4, uint32_t)
APERTURE_MATH_VEC4(float4, ApertureFloat4, float)

#define APERTURE_MATH_MATRIX_OPS(fn, Type, T, ColVec, RowVec, R, C)           \
  static inline T aperture_##fn##_get(const Type* m, size_t row,              \
                                      size_t col) {                           \
    APERTURE_C_ASSERT(m != NULL);                                             \
    APERTURE_C_ASSERT(row < (size_t)(R));                                     \
    APERTURE_C_ASSERT(col < (size_t)(C));                                     \
    return m->data[(col * (size_t)(R)) + row];                                \
  }                                                                           \
  static inline void aperture_##fn##_set(Type* m, size_t row, size_t col,     \
                                         T value) {                           \
    APERTURE_C_ASSERT(m != NULL);                                             \
    APERTURE_C_ASSERT(row < (size_t)(R));                                     \
    APERTURE_C_ASSERT(col < (size_t)(C));                                     \
    m->data[(col * (size_t)(R)) + row] = value;                               \
  }                                                                           \
  static inline ColVec aperture_##fn##_column(const Type* m, size_t col) {    \
    APERTURE_C_ASSERT(m != NULL);                                             \
    APERTURE_C_ASSERT(col < (size_t)(C));                                     \
    return m->columns[col];                                                   \
  }                                                                           \
  static inline void aperture_##fn##_set_column(Type* m, size_t col,          \
                                                ColVec vec) {                 \
    APERTURE_C_ASSERT(m != NULL);                                             \
    APERTURE_C_ASSERT(col < (size_t)(C));                                     \
    m->columns[col] = vec;                                                    \
  }                                                                           \
  static inline RowVec aperture_##fn##_row(const Type* m, size_t row) {       \
    APERTURE_C_ASSERT(m != NULL);                                             \
    APERTURE_C_ASSERT(row < (size_t)(R));                                     \
    RowVec vec;                                                               \
    for (size_t col = 0; col < (size_t)(C); ++col) {                          \
      vec.data[col] = m->data[(col * (size_t)(R)) + row];                     \
    }                                                                         \
    return vec;                                                               \
  }                                                                           \
  static inline void aperture_##fn##_set_row(Type* m, size_t row,             \
                                             RowVec vec) {                    \
    APERTURE_C_ASSERT(m != NULL);                                             \
    APERTURE_C_ASSERT(row < (size_t)(R));                                     \
    for (size_t col = 0; col < (size_t)(C); ++col) {                          \
      m->data[(col * (size_t)(R)) + row] = vec.data[col];                     \
    }                                                                         \
  }

/// @brief Indexed matrix element in column-major order.
/// @param m Matrix; must not be `NULL`
/// @param row Row index in `[0, R)`
/// @param col Column index in `[0, C)`
/// @return Element at (`row`, `col`)
/// @warning Asserts if `m` is null or the indices are out of range.
///
/// @brief Writes a matrix element in column-major order.
/// @param m Matrix; must not be `NULL`
/// @param row Row index in `[0, R)`
/// @param col Column index in `[0, C)`
/// @param value Element to store
/// @warning Asserts if `m` is null or the indices are out of range.
///
/// @brief Copies column `col` as a vector of length `R`.
/// @param m Matrix; must not be `NULL`
/// @param col Column index in `[0, C)`
/// @return Column vector
/// @warning Asserts if `m` is null or `col` is out of range.
///
/// @brief Replaces column `col` with `vec`.
/// @param m Matrix; must not be `NULL`
/// @param col Column index in `[0, C)`
/// @param vec Column vector of length `R`
/// @warning Asserts if `m` is null or `col` is out of range.
///
/// @brief Copies row `row` as a vector of length `C`.
/// @param m Matrix; must not be `NULL`
/// @param row Row index in `[0, R)`
/// @return Row vector
/// @warning Asserts if `m` is null or `row` is out of range.
///
/// @brief Replaces row `row` with `vec`.
/// @param m Matrix; must not be `NULL`
/// @param row Row index in `[0, R)`
/// @param vec Row vector of length `C`
/// @warning Asserts if `m` is null or `row` is out of range.
APERTURE_MATH_MATRIX_OPS(int2x2, ApertureInt2x2, int32_t, ApertureInt2,
                         ApertureInt2, 2, 2)
APERTURE_MATH_MATRIX_OPS(int2x3, ApertureInt2x3, int32_t, ApertureInt2,
                         ApertureInt3, 2, 3)
APERTURE_MATH_MATRIX_OPS(int2x4, ApertureInt2x4, int32_t, ApertureInt2,
                         ApertureInt4, 2, 4)
APERTURE_MATH_MATRIX_OPS(int3x2, ApertureInt3x2, int32_t, ApertureInt3,
                         ApertureInt2, 3, 2)
APERTURE_MATH_MATRIX_OPS(int3x3, ApertureInt3x3, int32_t, ApertureInt3,
                         ApertureInt3, 3, 3)
APERTURE_MATH_MATRIX_OPS(int3x4, ApertureInt3x4, int32_t, ApertureInt3,
                         ApertureInt4, 3, 4)
APERTURE_MATH_MATRIX_OPS(int4x2, ApertureInt4x2, int32_t, ApertureInt4,
                         ApertureInt2, 4, 2)
APERTURE_MATH_MATRIX_OPS(int4x3, ApertureInt4x3, int32_t, ApertureInt4,
                         ApertureInt3, 4, 3)
APERTURE_MATH_MATRIX_OPS(int4x4, ApertureInt4x4, int32_t, ApertureInt4,
                         ApertureInt4, 4, 4)
APERTURE_MATH_MATRIX_OPS(uint2x2, ApertureUint2x2, uint32_t, ApertureUint2,
                         ApertureUint2, 2, 2)
APERTURE_MATH_MATRIX_OPS(uint2x3, ApertureUint2x3, uint32_t, ApertureUint2,
                         ApertureUint3, 2, 3)
APERTURE_MATH_MATRIX_OPS(uint2x4, ApertureUint2x4, uint32_t, ApertureUint2,
                         ApertureUint4, 2, 4)
APERTURE_MATH_MATRIX_OPS(uint3x2, ApertureUint3x2, uint32_t, ApertureUint3,
                         ApertureUint2, 3, 2)
APERTURE_MATH_MATRIX_OPS(uint3x3, ApertureUint3x3, uint32_t, ApertureUint3,
                         ApertureUint3, 3, 3)
APERTURE_MATH_MATRIX_OPS(uint3x4, ApertureUint3x4, uint32_t, ApertureUint3,
                         ApertureUint4, 3, 4)
APERTURE_MATH_MATRIX_OPS(uint4x2, ApertureUint4x2, uint32_t, ApertureUint4,
                         ApertureUint2, 4, 2)
APERTURE_MATH_MATRIX_OPS(uint4x3, ApertureUint4x3, uint32_t, ApertureUint4,
                         ApertureUint3, 4, 3)
APERTURE_MATH_MATRIX_OPS(uint4x4, ApertureUint4x4, uint32_t, ApertureUint4,
                         ApertureUint4, 4, 4)
APERTURE_MATH_MATRIX_OPS(float2x2, ApertureFloat2x2, float, ApertureFloat2,
                         ApertureFloat2, 2, 2)
APERTURE_MATH_MATRIX_OPS(float2x3, ApertureFloat2x3, float, ApertureFloat2,
                         ApertureFloat3, 2, 3)
APERTURE_MATH_MATRIX_OPS(float2x4, ApertureFloat2x4, float, ApertureFloat2,
                         ApertureFloat4, 2, 4)
APERTURE_MATH_MATRIX_OPS(float3x2, ApertureFloat3x2, float, ApertureFloat3,
                         ApertureFloat2, 3, 2)
APERTURE_MATH_MATRIX_OPS(float3x3, ApertureFloat3x3, float, ApertureFloat3,
                         ApertureFloat3, 3, 3)
APERTURE_MATH_MATRIX_OPS(float3x4, ApertureFloat3x4, float, ApertureFloat3,
                         ApertureFloat4, 3, 4)
APERTURE_MATH_MATRIX_OPS(float4x2, ApertureFloat4x2, float, ApertureFloat4,
                         ApertureFloat2, 4, 2)
APERTURE_MATH_MATRIX_OPS(float4x3, ApertureFloat4x3, float, ApertureFloat4,
                         ApertureFloat3, 4, 3)
APERTURE_MATH_MATRIX_OPS(float4x4, ApertureFloat4x4, float, ApertureFloat4,
                         ApertureFloat4, 4, 4)

#undef APERTURE_MATH_VEC2
#undef APERTURE_MATH_VEC3
#undef APERTURE_MATH_VEC4
#undef APERTURE_MATH_MATRIX_OPS

// clang-format on

#ifdef _MSC_VER
#pragma warning(pop)
#elif defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
