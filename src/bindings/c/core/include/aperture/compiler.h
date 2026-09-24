#ifndef APERTURE_COMPILER_H
#define APERTURE_COMPILER_H

#if defined(__GNUC__) || defined(__clang__)
#define APERTURE_C_EXPECT_TRUE(x) __builtin_expect(!!(x), 1)
#define APERTURE_C_EXPECT_FALSE(x) __builtin_expect(!!(x), 0)
#else
#define APERTURE_C_EXPECT_TRUE(x) (x)
#define APERTURE_C_EXPECT_FALSE(x) (x)
#endif

#define APERTURE_C_CONCAT_IMPL(a, b) a##b
#define APERTURE_C_CONCAT(a, b) APERTURE_C_CONCAT_IMPL(a, b)
#define APERTURE_C_ANONYMOUS_VAR(prefix) APERTURE_C_CONCAT(prefix, __LINE__)

#if defined(__GNUC__) || defined(__clang__)
#define APERTURE_C_PRINTF(fmt_index, args_index) \
  __attribute__((format(printf, fmt_index, args_index)))
#else
#define APERTURE_C_PRINTF(fmt_index, args_index)
#endif

#ifdef _MSC_VER
#define APERTURE_C_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define APERTURE_C_FORCE_INLINE __attribute__((always_inline)) inline
#else
#define APERTURE_C_FORCE_INLINE inline
#endif

#ifdef _MSC_VER
#define APERTURE_C_NO_INLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define APERTURE_C_NO_INLINE __attribute__((noinline))
#else
#define APERTURE_C_NO_INLINE
#endif

#endif
