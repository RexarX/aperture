#include <pch.hpp>

#include <aperture/assert.hpp>

#include <aperture/log.hpp>
#include <aperture/utils/format.hpp>

#include <atomic>
#include <source_location>
#include <string>
#include <string_view>

namespace aperture {

namespace {

// Relaxed is sufficient: a plain function pointer has no associated data
// to synchronize, same reasoning as the logger's custom-callback storage.
std::atomic<AssertionHandler> g_custom_assertion_handler{nullptr};

[[nodiscard]] constexpr std::string_view GetFileName(std::string_view path) {
  const size_t last_slash = path.find_last_of("/\\");
  return (last_slash != std::string_view::npos) ? path.substr(last_slash + 1)
                                                : path;
}

// Implementation detail of HandleAssertion — never declared in the public
// header, so nothing outside this translation unit can see or call it.
void DefaultAssertionHandler(std::string_view condition,
                             std::string_view message,
                             const std::source_location& loc) noexcept {
  const auto formatted = FormatAssertionMessage(condition, message, loc);
  // Already fully formatted, so use the string_view overload directly
  // rather than routing back through a format string.
  log::Critical(std::string_view{formatted});
}

}  // namespace

std::string FormatAssertionMessage(std::string_view condition,
                                   std::string_view message,
                                   const std::source_location& loc) {
  std::string result;
  result.reserve(256);

  if (!message.empty()) {
    utils::FormatTo(result, "Assertion failed: {} | {}", condition, message);
  } else {
    utils::FormatTo(result, "Assertion failed: {}", condition);
  }

  const std::string_view filename = GetFileName(loc.file_name());
  utils::FormatTo(result, " [{}:{}]", filename, loc.line());

  return result;
}

void HandleAssertion(std::string_view condition, std::string_view message,
                     const std::source_location& loc) noexcept {
  // Priority 1: Custom user handler
  if (AssertionHandler custom =
          g_custom_assertion_handler.load(std::memory_order_relaxed)) {
    custom(condition, message, loc);
    return;
  }

  // Priority 2: Default handler (routes through aperture::log::Critical)
  DefaultAssertionHandler(condition, message, loc);
}

void SetAssertionHandler(AssertionHandler handler) noexcept {
  g_custom_assertion_handler.store(handler, std::memory_order_relaxed);
}

AssertionHandler GetAssertionHandler() noexcept {
  return g_custom_assertion_handler.load(std::memory_order_relaxed);
}

}  // namespace aperture
