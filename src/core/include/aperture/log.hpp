#pragma once

#include <aperture/platform.hpp>
#include <aperture/utils/format.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <memory_resource>
#include <string_view>
#include <utility>

namespace aperture::log {

/// @brief Log severity levels.
enum class Level : uint8_t { Trace, Debug, Info, Warn, Error, Critical };

/// @brief Converts a log level to its string representation.
/// @param level The log level to convert
/// @return String representation of the log level
[[nodiscard]] constexpr std::string_view ToString(Level level) noexcept {
  switch (level) {
    using enum Level;
    case Trace:
      return "trace";
    case Debug:
      return "debug";
    case Info:
      return "info";
    case Warn:
      return "warn";
    case Error:
      return "error";
    case Critical:
      return "critical";
  }
  return "unknown";
}

/// @brief Signature of a user-supplied logger callback.
using Callback = void (*)(Level level, std::string_view message) noexcept;

/// @brief Installs a custom logger, overriding the built-in output.
/// Passing nullptr is equivalent to calling ResetCustomLogger().
/// @param callback Function pointer to receive all future log records.
APERTURE_API void SetCustomLogger(Callback callback) noexcept;

/// @brief Removes any installed custom logger, restoring built-in output.
APERTURE_API void ResetCustomLogger() noexcept;

/// @brief Checks whether a custom logger is currently installed.
/// @return `true` if a custom logger is installed, `false` otherwise
[[nodiscard]] APERTURE_API bool HasCustomLogger() noexcept;

/// @brief Dispatches a pre-formatted message to the active logger.
/// @param level The severity level of the log message
/// @param message The pre-formatted log message
APERTURE_API void Log(Level level, std::string_view message) noexcept;

/// @brief Size of the inline buffer used for formatting log messages.
inline constexpr size_t INLINE_BUFFER_SIZE = 256;

/// @brief Logs a formatted message with the specified severity level.
/// @tparam Args Types of the format arguments
/// @param level The severity level of the log message
/// @param fmt Format string
/// @param args Arguments for the format string
template <typename... Args>
inline void Log(Level level, std::format_string<Args...> fmt,
                Args&&... args) noexcept {
  std::array<std::byte, INLINE_BUFFER_SIZE> stack_buffer = {};
  std::pmr::monotonic_buffer_resource resource{stack_buffer.data(),
                                               stack_buffer.size(),
                                               std::pmr::new_delete_resource()};
  std::pmr::string message{&resource};
  utils::FormatTo(message, fmt, std::forward<Args>(args)...);
  Log(level, message);
}

#ifndef NDEBUG
/// @brief Logs a trace message.
/// @param message The message to log
inline void Trace(std::string_view message) noexcept {
  Log(Level::Trace, message);
}

/// @brief Logs a formatted trace message.
/// @tparam Args Types of the format arguments
/// @param fmt Format string
/// @param args Arguments for the format string
template <typename... Args>
  requires(sizeof...(Args) > 0)
inline void Trace(std::format_string<Args...> fmt, Args&&... args) noexcept {
  Log(Level::Trace, fmt, std::forward<Args>(args)...);
}

/// @brief Logs a debug message.
/// @param message The message to log
inline void Debug(std::string_view message) noexcept {
  Log(Level::Debug, message);
}

/// @brief Logs a formatted debug message.
/// @tparam Args Types of the format arguments
/// @param fmt Format string
/// @param args Arguments for the format string
template <typename... Args>
  requires(sizeof...(Args) > 0)
inline void Debug(std::format_string<Args...> fmt, Args&&... args) noexcept {
  Log(Level::Debug, fmt, std::forward<Args>(args)...);
}

#else
inline void Trace(std::string_view /*message*/) noexcept {}

template <typename... Args>
inline void Trace(Args&&... /*args*/) noexcept {}

inline void Debug(std::string_view /*message*/) noexcept {}

template <typename... Args>
inline void Debug(Args&&... /*args*/) noexcept {}
#endif

/// @brief Logs an info message.
/// @param message The message to log
inline void Info(std::string_view message) noexcept {
  Log(Level::Info, message);
}

/// @brief Logs a formatted info message.
/// @tparam Args Types of the format arguments
/// @param fmt Format string
/// @param args Arguments for the format string
template <typename... Args>
  requires(sizeof...(Args) > 0)
inline void Info(std::format_string<Args...> fmt, Args&&... args) noexcept {
  Log(Level::Info, fmt, std::forward<Args>(args)...);
}

/// @brief Logs a warning message.
/// @param message The message to log
inline void Warn(std::string_view message) noexcept {
  Log(Level::Warn, message);
}

/// @brief Logs a formatted warning message.
/// @tparam Args Types of the format arguments
/// @param fmt Format string
/// @param args Arguments for the format string
template <typename... Args>
  requires(sizeof...(Args) > 0)
inline void Warn(std::format_string<Args...> fmt, Args&&... args) noexcept {
  Log(Level::Warn, fmt, std::forward<Args>(args)...);
}

/// @brief Logs an error message.
/// @param message The message to log
inline void Error(std::string_view message) noexcept {
  Log(Level::Error, message);
}

/// @brief Logs a formatted error message.
/// @tparam Args Types of the format arguments
/// @param fmt Format string
/// @param args Arguments for the format string
template <typename... Args>
  requires(sizeof...(Args) > 0)
inline void Error(std::format_string<Args...> fmt, Args&&... args) noexcept {
  Log(Level::Error, fmt, std::forward<Args>(args)...);
}

/// @brief Logs a critical message.
/// @param message The message to log
inline void Critical(std::string_view message) noexcept {
  Log(Level::Critical, message);
}

/// @brief Logs a formatted critical message.
/// @tparam Args Types of the format arguments
/// @param fmt Format string
/// @param args Arguments for the format string
template <typename... Args>
  requires(sizeof...(Args) > 0)
inline void Critical(std::format_string<Args...> fmt, Args&&... args) noexcept {
  Log(Level::Critical, fmt, std::forward<Args>(args)...);
}

}  // namespace aperture::log
