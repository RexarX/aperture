#pragma once

#include <aperture/compiler.hpp>
#include <aperture/platform.hpp>
#include <aperture/utils/macro.hpp>

#include <format>
#include <source_location>
#include <string>
#include <string_view>

namespace aperture {

/**
 * @brief Function signature for custom assertion handlers.
 * @param condition The failed condition as a string
 * @param message Additional message (may be empty)
 * @param loc Source location of the assertion
 */
using AssertionHandler = void (*)(std::string_view condition,
                                  std::string_view message,
                                  const std::source_location& loc) noexcept;

/// @brief Default assertion handler (`nullptr` means use built-in default
/// behavior).
inline constexpr AssertionHandler kDefaultAssertionHandler = nullptr;

namespace details {

#if !defined(NDEBUG) || defined(APERTURE_ENABLE_ASSERT)
inline constexpr bool kEnableAssert = true;
#else
inline constexpr bool kEnableAssert = false;
#endif

}  // namespace details

/**
 * @brief Formats a complete assertion failure message with source location.
 * @details Exposed publicly so a custom assertion handler (see
 * SetAssertionHandler) can reuse the same "condition | message [file:line]"
 * formatting the built-in default handler uses, rather than reimplementing
 * it.
 * @param condition The failed condition as a string
 * @param message Additional message (may be empty)
 * @param loc Source location of the assertion
 * @return Formatted assertion message
 */
[[nodiscard]] APERTURE_API std::string FormatAssertionMessage(
    std::string_view condition, std::string_view message,
    const std::source_location& loc = std::source_location::current());

/**
 * @brief Unified assertion handling function.
 * @details Priority order:
 *   1. Custom handler (if set by user via SetAssertionHandler)
 *   2. Default handler (routes through aperture::log::Critical)
 *
 * @param condition The failed condition as a string
 * @param message Additional message
 * @param loc Source location of the assertion
 */
APERTURE_API void HandleAssertion(
    std::string_view condition, std::string_view message,
    const std::source_location& loc = std::source_location::current()) noexcept;

/**
 * @brief Sets a custom assertion handler.
 * @details When set, this handler is called for all assertion failures instead
 * of the default behavior. Set to `nullptr` to restore default behavior.
 *
 * Priority order for assertion handling:
 *   1. Custom handler (if set via this function)
 *   2. Default handler (routes through aperture::log::Critical)
 *
 * @param handler The custom handler function, or `nullptr` to use default
 *
 * @code
 * // Set custom handler
 * aperture::SetAssertionHandler(
 *     [](std::string_view condition, std::string_view message,
 *        const std::source_location& loc) noexcept {
 *   // Reuse the built-in formatting, route it through your own logger
 *   mylib::log::Critical(aperture::FormatAssertionMessage(condition, message,
 * loc));
 * });
 *
 * // Reset to default behavior
 * aperture::SetAssertionHandler(aperture::kDefaultAssertionHandler);
 * @endcode
 */
APERTURE_API void SetAssertionHandler(AssertionHandler handler) noexcept;

/**
 * @brief Gets the current custom assertion handler.
 * @return The current custom handler, or `nullptr` if using default behavior
 */
[[nodiscard]] APERTURE_API AssertionHandler GetAssertionHandler() noexcept;

}  // namespace aperture

// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)
// NOLINTBEGIN(cppcoreguidelines-macro-usage)

/**
 * @brief Assertion macro that aborts execution in debug builds.
 * @details Does nothing in release builds.
 * Uses the configured assertion handler.
 * Supports format strings and arguments.
 * @param condition The condition to check
 * @param ... Optional message (can be format string with arguments)
 * @hideinitializer
 */
#if !defined(NDEBUG) || defined(APERTURE_ENABLE_ASSERT)
#define APERTURE_ASSERT(condition, ...)                                   \
  do {                                                                    \
    if constexpr (::aperture::details::kEnableAssert) {                   \
      if (APERTURE_EXPECT_FALSE(!(condition))) [[unlikely]] {             \
        if constexpr (sizeof(#__VA_ARGS__) > 1) {                         \
          try {                                                           \
            const auto msg = std::format("" __VA_ARGS__);                 \
            ::aperture::HandleAssertion(#condition, msg);                 \
          } catch (...) {                                                 \
            ::aperture::HandleAssertion(#condition,                       \
                                        "Formatting error in assertion"); \
          }                                                               \
        } else {                                                          \
          ::aperture::HandleAssertion(#condition, "");                    \
        }                                                                 \
        APERTURE_DEBUG_BREAK();                                           \
      }                                                                   \
    }                                                                     \
  } while (false)
#else
#define APERTURE_ASSERT(condition, ...)                          \
  [[maybe_unused]] static constexpr auto APERTURE_ANONYMOUS_VAR( \
      unused_assert) = 0
#endif

/**
 * @brief Invariant check that asserts in debug builds and logs error in
 * release.
 * @details Provides runtime safety checks that are enforced even in release
 * builds. In debug builds, triggers assertion. In release builds, logs error
 * and continues.
 * @param condition The condition to check
 * @param ... Optional message (can be format string with arguments)
 * @hideinitializer
 */
#if !defined(NDEBUG) || defined(APERTURE_ENABLE_ASSERT)
#define APERTURE_INVARIANT(condition, ...)                              \
  do {                                                                  \
    if (APERTURE_EXPECT_FALSE(!(condition))) [[unlikely]] {             \
      if constexpr (sizeof(#__VA_ARGS__) > 1) {                         \
        try {                                                           \
          const auto msg = std::format("" __VA_ARGS__);                 \
          ::aperture::HandleAssertion(#condition, msg);                 \
        } catch (...) {                                                 \
          ::aperture::HandleAssertion(#condition,                       \
                                      "Formatting error in invariant"); \
        }                                                               \
      } else {                                                          \
        ::aperture::HandleAssertion(#condition, "");                    \
      }                                                                 \
      APERTURE_DEBUG_BREAK();                                           \
    }                                                                   \
  } while (false)
#else
#define APERTURE_INVARIANT(condition, ...)                  \
  do {                                                      \
    if (APERTURE_EXPECT_FALSE(!(condition))) [[unlikely]] { \
      if constexpr (sizeof(#__VA_ARGS__) > 1) {             \
        try {                                               \
          const auto msg = std::format("" __VA_ARGS__);     \
          ::aperture::HandleAssertion(#condition, msg);     \
        } catch (...) {                                     \
          ::aperture::HandleAssertion(#condition, "");      \
        }                                                   \
      } else {                                              \
        ::aperture::HandleAssertion(#condition, "");        \
      }                                                     \
    }                                                       \
  } while (false)
#endif

/**
 * @brief Verify macro that always checks the condition.
 * @details Similar to assert but runs in both debug and release builds.
 * Useful for validating external input or critical invariants.
 * @param condition The condition to check
 * @param ... Optional message (can be format string with arguments)
 * @hideinitializer
 */
#define APERTURE_VERIFY(condition, ...)                              \
  do {                                                               \
    if (APERTURE_EXPECT_FALSE(!(condition))) [[unlikely]] {          \
      if constexpr (sizeof(#__VA_ARGS__) > 1) {                      \
        try {                                                        \
          const auto msg = std::format("" __VA_ARGS__);              \
          ::aperture::HandleAssertion(#condition, msg);              \
        } catch (...) {                                              \
          ::aperture::HandleAssertion(#condition,                    \
                                      "Formatting error in verify"); \
        }                                                            \
      } else {                                                       \
        ::aperture::HandleAssertion(#condition, "");                 \
      }                                                              \
      APERTURE_DEBUG_BREAK();                                        \
    }                                                                \
  } while (false)

// NOLINTEND(cppcoreguidelines-macro-usage)
// NOLINTEND(cppcoreguidelines-avoid-do-while)
