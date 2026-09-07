#include <pch.hpp>

#include <aperture/log.hpp>

#include <atomic>
#include <cstdio>
#include <string_view>

#if defined(__cpp_lib_print) && (__cpp_lib_print >= 202302L)
#include <print>
#define APERTURE_HAS_STD_PRINT 1
#else
#define APERTURE_HAS_STD_PRINT 0
#endif

namespace aperture::log {

namespace {

// Relaxed is sufficient: the callback is a plain function pointer (no
// associated data to synchronize), so there's nothing that requires
// acquire/release ordering when it's read on another thread.
std::atomic<Callback> g_custom_logger{nullptr};

void DefaultLog(Level level, std::string_view message) noexcept {
  std::FILE* stream = level >= Level::kWarn ? stderr : stdout;
  const std::string_view level_str = ToString(level);

#if APERTURE_HAS_STD_PRINT
  std::println(stream, "[{}] {}", level_str, message);
#else
  std::fprintf(stream, "[%.*s] %.*s\n", static_cast<int>(level_str.size()),
               level_str.data(), static_cast<int>(message.size()),
               message.data());
#endif
}

}  // namespace

void SetCustomLogger(Callback callback) noexcept {
  g_custom_logger.store(callback, std::memory_order_relaxed);
}

void ResetCustomLogger() noexcept {
  g_custom_logger.store(nullptr, std::memory_order_relaxed);
}

bool HasCustomLogger() noexcept {
  return g_custom_logger.load(std::memory_order_relaxed) != nullptr;
}

void Log(Level level, std::string_view message) noexcept {
  if (Callback custom = g_custom_logger.load(std::memory_order_relaxed)) {
    custom(level, message);
    return;
  }
  DefaultLog(level, message);
}

}  // namespace aperture::log
