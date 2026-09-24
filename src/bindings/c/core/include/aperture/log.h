#ifndef APERTURE_LOG_H
#define APERTURE_LOG_H

#include <aperture/compiler.h>
#include <aperture/platform.h>

#include <stdbool.h>
#include <stdint.h>

APERTURE_C_BEGIN

/// @brief Log severity levels.
typedef uint8_t ApertureLogLevel;

enum {
  APERTURE_LOG_LEVEL_TRACE = 0U,
  APERTURE_LOG_LEVEL_DEBUG,
  APERTURE_LOG_LEVEL_INFO,
  APERTURE_LOG_LEVEL_WARN,
  APERTURE_LOG_LEVEL_ERROR,
  APERTURE_LOG_LEVEL_CRITICAL,
};

/// @brief Converts a log level to its string representation.
/// @param level Enumerator to convert
/// @return Enumerator name, or `"unknown"`
static inline const char* aperture_log_level_to_string(ApertureLogLevel level) {
  switch (level) {
    case APERTURE_LOG_LEVEL_TRACE:
      return "trace";
    case APERTURE_LOG_LEVEL_DEBUG:
      return "debug";
    case APERTURE_LOG_LEVEL_INFO:
      return "info";
    case APERTURE_LOG_LEVEL_WARN:
      return "warn";
    case APERTURE_LOG_LEVEL_ERROR:
      return "error";
    case APERTURE_LOG_LEVEL_CRITICAL:
      return "critical";
    default:
      return "unknown";
  }
}

/// @brief Signature of a user-supplied logger callback.
/// @details `message` is NUL-terminated. The callback must be thread-safe and
/// must not call aperture.
/// @param level Severity of the record
/// @param message NUL-terminated text
/// @param user Pointer passed to `aperture_log_set_custom`
typedef void (*ApertureLogCallback)(ApertureLogLevel level, const char* message,
                                    void* user);

/// @brief Installs a custom C logger, overriding the built-in output.
/// @details Wraps `aperture::log::SetCustomLogger` with a thunk that
/// forwards to `callback`. Passing `NULL` is equivalent to
/// `aperture_log_reset_custom()`.
/// @param callback Function pointer to receive all future log records, or
/// `NULL` for default
/// @param user Forwarded to every callback invocation. May be `NULL`
APERTURE_C_API void aperture_log_set_custom(ApertureLogCallback callback,
                                            void* user) APERTURE_C_NOEXCEPT;

/// @brief Removes any installed custom logger, restoring built-in output.
/// @details Wraps `aperture::log::ResetCustomLogger()`. Equivalent to
/// `aperture_log_set_custom(NULL, NULL)`.
APERTURE_C_API void aperture_log_reset_custom(void) APERTURE_C_NOEXCEPT;

/// @brief Checks whether a custom C logger is currently installed.
/// @return `true` if the C++ slot holds the C thunk, `false` otherwise
APERTURE_C_API bool aperture_log_has_custom(void) APERTURE_C_NOEXCEPT;

/// @brief Dispatches a pre-formatted NUL-terminated message to the active
/// logger.
/// @param level Severity of the record
/// @param message NUL-terminated text
/// @warning Asserts if `message` is null.
APERTURE_C_API void aperture_log(ApertureLogLevel level,
                                 const char* message) APERTURE_C_NOEXCEPT;

/// @brief Logs a NUL-terminated trace message. No-op in release.
/// @param message NUL-terminated text
/// @warning Asserts if `message` is null when logging is enabled.
APERTURE_C_API void aperture_log_trace(const char* message) APERTURE_C_NOEXCEPT;

/// @brief Logs a NUL-terminated debug message. No-op in release.
/// @param message NUL-terminated text
/// @warning Asserts if `message` is null when logging is enabled.
APERTURE_C_API void aperture_log_debug(const char* message) APERTURE_C_NOEXCEPT;

/// @brief Logs a NUL-terminated info message.
/// @param message NUL-terminated text
/// @warning Asserts if `message` is null.
APERTURE_C_API void aperture_log_info(const char* message) APERTURE_C_NOEXCEPT;

/// @brief Logs a NUL-terminated warning message.
/// @param message NUL-terminated text
/// @warning Asserts if `message` is null.
APERTURE_C_API void aperture_log_warn(const char* message) APERTURE_C_NOEXCEPT;

/// @brief Logs a NUL-terminated error message.
/// @param message NUL-terminated text
/// @warning Asserts if `message` is null.
APERTURE_C_API void aperture_log_error(const char* message) APERTURE_C_NOEXCEPT;

/// @brief Logs a NUL-terminated critical message.
/// @param message NUL-terminated text
/// @warning Asserts if `message` is null.
APERTURE_C_API void aperture_log_critical(const char* message)
    APERTURE_C_NOEXCEPT;

/// @brief Logs a `printf`-formatted message at `level`.
/// @param level Severity of the record
/// @param fmt `printf` format string
/// @warning Asserts if `fmt` is null.
APERTURE_C_API void aperture_logf(ApertureLogLevel level, const char* fmt,
                                  ...) APERTURE_C_NOEXCEPT
    APERTURE_C_PRINTF(2, 3);

/// @brief Logs a `printf`-formatted trace message. No-op in release.
/// @param fmt `printf` format string
/// @warning Asserts if `fmt` is null when logging is enabled.
APERTURE_C_API void aperture_log_tracef(const char* fmt,
                                        ...) APERTURE_C_NOEXCEPT
    APERTURE_C_PRINTF(1, 2);

/// @brief Logs a `printf`-formatted debug message. No-op in release.
/// @param fmt `printf` format string
/// @warning Asserts if `fmt` is null when logging is enabled.
APERTURE_C_API void aperture_log_debugf(const char* fmt,
                                        ...) APERTURE_C_NOEXCEPT
    APERTURE_C_PRINTF(1, 2);

/// @brief Logs a `printf`-formatted info message.
/// @param fmt `printf` format string
/// @warning Asserts if `fmt` is null.
APERTURE_C_API void aperture_log_infof(const char* fmt, ...) APERTURE_C_NOEXCEPT
    APERTURE_C_PRINTF(1, 2);

/// @brief Logs a `printf`-formatted warning message.
/// @param fmt `printf` format string
/// @warning Asserts if `fmt` is null.
APERTURE_C_API void aperture_log_warnf(const char* fmt, ...) APERTURE_C_NOEXCEPT
    APERTURE_C_PRINTF(1, 2);

/// @brief Logs a `printf`-formatted error message.
/// @param fmt `printf` format string
/// @warning Asserts if `fmt` is null.
APERTURE_C_API void aperture_log_errorf(const char* fmt,
                                        ...) APERTURE_C_NOEXCEPT
    APERTURE_C_PRINTF(1, 2);

/// @brief Logs a `printf`-formatted critical message.
/// @param fmt `printf` format string
/// @warning Asserts if `fmt` is null.
APERTURE_C_API void aperture_log_criticalf(const char* fmt,
                                           ...) APERTURE_C_NOEXCEPT
    APERTURE_C_PRINTF(1, 2);

APERTURE_C_END

#endif
