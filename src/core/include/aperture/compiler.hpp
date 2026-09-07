#pragma once

#include <version>

#if defined(__GNUC__) || defined(__clang__)
#define APERTURE_EXPECT_TRUE(x) __builtin_expect(!!(x), 1)
#define APERTURE_EXPECT_FALSE(x) __builtin_expect(!!(x), 0)
#else
#define APERTURE_EXPECT_TRUE(x) (x)
#define APERTURE_EXPECT_FALSE(x) (x)
#endif

#ifdef _MSC_VER
#define APERTURE_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define APERTURE_FORCE_INLINE __attribute__((always_inline)) inline
#else
#define APERTURE_FORCE_INLINE inline
#endif

#ifdef __clang__
#define APERTURE_ALWAYS_INLINE [[clang::always_inline]]
#elif defined(__GNUC__)
#define APERTURE_ALWAYS_INLINE [[gnu::always_inline]]
#elif defined(_MSC_VER)
#define APERTURE_ALWAYS_INLINE [[msvc::forceinline]]
#else
#define APERTURE_ALWAYS_INLINE
#endif

#ifdef _MSC_VER
#define APERTURE_NO_INLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define APERTURE_NO_INLINE __attribute__((noinline))
#else
#define APERTURE_NO_INLINE
#endif

// MSVC and clang-cl ignore standard [[no_unique_address]] under the MSVC ABI.
#ifdef _MSC_VER
#define APERTURE_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#define APERTURE_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif
