#include <aperture/log.h>

#include <aperture/assert.hpp>
#include <aperture/log.hpp>

#include <array>
#include <atomic>
#include <cstdarg>
#include <cstdbool>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>

namespace {

std::atomic<ApertureLogCallback> g_c_callback{nullptr};
std::atomic<void*> g_c_user{nullptr};

void CThunk(aperture::log::Level level, std::string_view message) noexcept {
  const ApertureLogCallback callback =
      g_c_callback.load(std::memory_order_relaxed);
  if (callback == nullptr) {
    return;
  }
  void* user = g_c_user.load(std::memory_order_relaxed);

  const auto c_level = static_cast<ApertureLogLevel>(level);
  if (message.empty()) {
    callback(c_level, "", user);
    return;
  }

  std::array<char, aperture::log::INLINE_BUFFER_SIZE> stack = {};
  if (message.size() + 1 <= stack.size()) {
    std::memcpy(stack.data(), message.data(), message.size());
    stack[message.size()] = '\0';
    callback(c_level, stack.data(), user);
    return;
  }

  std::string owned(message);
  callback(c_level, owned.c_str(), user);
}

void VLog(aperture::log::Level level, const char* fmt, va_list args) noexcept {
  APERTURE_ASSERT(fmt != nullptr);

  std::array<char, aperture::log::INLINE_BUFFER_SIZE> stack = {};
  va_list args_copy;
  va_copy(args_copy, args);
  const int written =
      std::vsnprintf(stack.data(), stack.size(), fmt, args_copy);
  va_end(args_copy);
  if (written < 0) {
    aperture::log::Log(level, fmt);
    return;
  }
  if (static_cast<size_t>(written) < stack.size()) {
    aperture::log::Log(level, stack.data());
    return;
  }

  std::string owned(static_cast<size_t>(written), '\0');
  std::vsnprintf(owned.data(), owned.size() + 1, fmt, args);
  aperture::log::Log(level, owned);
}

}  // namespace

extern "C" {

void aperture_log_set_custom(ApertureLogCallback callback,
                             void* user) noexcept {
  g_c_user.store(callback != nullptr ? user : nullptr,
                 std::memory_order_relaxed);
  g_c_callback.store(callback, std::memory_order_relaxed);
  aperture::log::SetCustomLogger(callback != nullptr ? &CThunk : nullptr);
}

void aperture_log_reset_custom(void) noexcept {
  aperture_log_set_custom(nullptr, nullptr);
}

bool aperture_log_has_custom(void) noexcept {
  return aperture::log::GetCustomLogger() == &CThunk;
}

void aperture_log(ApertureLogLevel level, const char* message) noexcept {
  APERTURE_ASSERT(message != nullptr);
  aperture::log::Log(static_cast<aperture::log::Level>(level), message);
}

void aperture_log_trace(const char* message) noexcept {
  APERTURE_ASSERT(message != nullptr);
  aperture::log::Trace(message);
}

void aperture_log_debug(const char* message) noexcept {
  APERTURE_ASSERT(message != nullptr);
  aperture::log::Debug(message);
}

void aperture_log_info(const char* message) noexcept {
  APERTURE_ASSERT(message != nullptr);
  aperture::log::Info(message);
}

void aperture_log_warn(const char* message) noexcept {
  APERTURE_ASSERT(message != nullptr);
  aperture::log::Warn(message);
}

void aperture_log_error(const char* message) noexcept {
  APERTURE_ASSERT(message != nullptr);
  aperture::log::Error(message);
}

void aperture_log_critical(const char* message) noexcept {
  APERTURE_ASSERT(message != nullptr);
  aperture::log::Critical(message);
}

void aperture_logf(ApertureLogLevel level, const char* fmt, ...) noexcept {
  va_list args;
  va_start(args, fmt);
  VLog(static_cast<aperture::log::Level>(level), fmt, args);
  va_end(args);
}

void aperture_log_tracef([[maybe_unused]] const char* fmt, ...) noexcept {
#ifndef NDEBUG
  va_list args;
  va_start(args, fmt);
  VLog(aperture::log::Level::Trace, fmt, args);
  va_end(args);
#endif
}

void aperture_log_debugf([[maybe_unused]] const char* fmt, ...) noexcept {
#ifndef NDEBUG
  va_list args;
  va_start(args, fmt);
  VLog(aperture::log::Level::Debug, fmt, args);
  va_end(args);
#endif
}

void aperture_log_infof(const char* fmt, ...) noexcept {
  va_list args;
  va_start(args, fmt);
  VLog(aperture::log::Level::Info, fmt, args);
  va_end(args);
}

void aperture_log_warnf(const char* fmt, ...) noexcept {
  va_list args;
  va_start(args, fmt);
  VLog(aperture::log::Level::Warn, fmt, args);
  va_end(args);
}

void aperture_log_errorf(const char* fmt, ...) noexcept {
  va_list args;
  va_start(args, fmt);
  VLog(aperture::log::Level::Error, fmt, args);
  va_end(args);
}

void aperture_log_criticalf(const char* fmt, ...) noexcept {
  va_list args;
  va_start(args, fmt);
  VLog(aperture::log::Level::Critical, fmt, args);
  va_end(args);
}
}
