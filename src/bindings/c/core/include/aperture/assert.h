#ifndef APERTURE_ASSERT_H
#define APERTURE_ASSERT_H

#include <aperture/compiler.h>
#include <aperture/platform.h>

#include <stddef.h>

APERTURE_C_BEGIN

/// @brief Function signature for a custom C assertion handler.
/// @param condition Failed condition as a string
/// @param message Additional message; may be empty but not `NULL`
/// @param file Source file of the assertion
/// @param line Source line of the assertion
typedef void (*ApertureAssertionHandler)(const char* condition,
                                         const char* message, const char* file,
                                         int line);

/// @brief Default C assertion handler (`NULL` means built-in critical log).
#define APERTURE_DEFAULT_ASSERTION_HANDLER ((ApertureAssertionHandler)NULL)

/// @brief Dispatches a failed C assertion to the active handler.
/// @details Calls the C handler when the C++ slot holds the C thunk,
/// otherwise the built-in critical-log format. Does not go through
/// `aperture::HandleAssertion` because C cannot provide
/// `std::source_location`.
/// @param condition Failed condition as a string
/// @param message Additional message, or `NULL` for none
/// @param file Source file of the assertion
/// @param line Source line of the assertion
APERTURE_C_API void aperture_handle_assertion(const char* condition,
                                              const char* message,
                                              const char* file,
                                              int line) APERTURE_C_NOEXCEPT;

/// @brief Installs a custom C assertion handler.
/// @details Wraps `aperture::SetAssertionHandler` with a thunk that
/// forwards to `handler`. Passing `NULL` restores the built-in critical
/// log.
/// @param handler Custom handler, or `NULL` for default
APERTURE_C_API void aperture_set_assertion_handler(
    ApertureAssertionHandler handler) APERTURE_C_NOEXCEPT;

/// @brief Current custom C assertion handler.
/// @return Custom handler if the C++ slot holds the C thunk, otherwise `NULL`
APERTURE_C_API ApertureAssertionHandler aperture_get_assertion_handler(void)
    APERTURE_C_NOEXCEPT;

/// @brief Assertion that is active in Debug and RelWithDebInfo.
/// @details No-op when `NDEBUG` is set and `APERTURE_ENABLE_ASSERT` is not.
/// Does not abort; after the handler it issues `APERTURE_C_DEBUG_BREAK()`.
/// @param condition Expression that must be true
/// @warning Fires when `condition` is false.
#if !defined(NDEBUG) || defined(APERTURE_ENABLE_ASSERT)
#define APERTURE_C_ASSERT(condition)                                 \
  do {                                                               \
    if (APERTURE_C_EXPECT_FALSE(!(condition))) {                     \
      aperture_handle_assertion(#condition, "", __FILE__, __LINE__); \
      APERTURE_C_DEBUG_BREAK();                                      \
    }                                                                \
  } while (0)

/// @brief Assertion with an explicit message string.
/// @param condition Expression that must be true
/// @param message NUL-terminated explanation, or `NULL`
/// @warning Fires when `condition` is false.
#define APERTURE_C_ASSERT_MSG(condition, message)                           \
  do {                                                                      \
    if (APERTURE_C_EXPECT_FALSE(!(condition))) {                            \
      aperture_handle_assertion(#condition, (message), __FILE__, __LINE__); \
      APERTURE_C_DEBUG_BREAK();                                             \
    }                                                                       \
  } while (0)
#else
#define APERTURE_C_ASSERT(condition) ((void)0)
#define APERTURE_C_ASSERT_MSG(condition, message) ((void)0)
#endif

APERTURE_C_END

#endif
