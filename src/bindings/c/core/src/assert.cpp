#include <aperture/assert.h>

#include <aperture/assert.hpp>
#include <aperture/log.hpp>
#include <aperture/utils/format.hpp>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstring>
#include <source_location>
#include <string>
#include <string_view>

namespace {

constexpr size_t CSTRING_BUFFER_SIZE = 256;

std::atomic<ApertureAssertionHandler> g_c_assertion_handler{nullptr};

[[nodiscard]] constexpr std::string_view FileName(
    std::string_view path) noexcept {
  const size_t last_slash = path.find_last_of("/\\");
  return (last_slash != std::string_view::npos) ? path.substr(last_slash + 1)
                                                : path;
}

[[nodiscard]] const char* CString(std::string_view text,
                                  std::array<char, CSTRING_BUFFER_SIZE>* stack,
                                  std::string* owned) noexcept {
  if (text.empty()) {
    return "";
  }
  if (text.size() + 1 <= stack->size()) {
    std::memcpy(stack->data(), text.data(), text.size());
    (*stack)[text.size()] = '\0';
    return stack->data();
  }
  owned->assign(text);
  return owned->c_str();
}

void CppToCAssertionThunk(std::string_view condition, std::string_view message,
                          const std::source_location& loc) noexcept {
  const ApertureAssertionHandler handler =
      g_c_assertion_handler.load(std::memory_order_relaxed);
  if (handler == nullptr) {
    return;
  }

  std::array<char, CSTRING_BUFFER_SIZE> condition_stack = {};
  std::array<char, CSTRING_BUFFER_SIZE> message_stack = {};
  std::string condition_owned;
  std::string message_owned;
  const char* file = loc.file_name() != nullptr ? loc.file_name() : "";
  handler(CString(condition, &condition_stack, &condition_owned),
          CString(message, &message_stack, &message_owned), file,
          static_cast<int>(loc.line()));
}

}  // namespace

extern "C" {

void aperture_handle_assertion(const char* condition, const char* message,
                               const char* file, int line) noexcept {
  const char* cond = condition != nullptr ? condition : "";
  const char* msg = message != nullptr ? message : "";
  const char* path = file != nullptr ? file : "";

  if (aperture::GetAssertionHandler() == &CppToCAssertionThunk) {
    const ApertureAssertionHandler handler =
        g_c_assertion_handler.load(std::memory_order_relaxed);
    if (handler != nullptr) {
      handler(cond, msg, path, line);
    }
    return;
  }

  std::string formatted;
  formatted.reserve(256);
  if (msg[0] != '\0') {
    aperture::utils::FormatTo(formatted, "Assertion failed: {} | {}", cond,
                              msg);
  } else {
    aperture::utils::FormatTo(formatted, "Assertion failed: {}", cond);
  }
  aperture::utils::FormatTo(formatted, " [{}:{}]", FileName(path), line);
  aperture::log::Critical(formatted);
}

void aperture_set_assertion_handler(ApertureAssertionHandler handler) noexcept {
  g_c_assertion_handler.store(handler, std::memory_order_relaxed);
  aperture::SetAssertionHandler(handler != nullptr ? &CppToCAssertionThunk
                                                   : nullptr);
}

ApertureAssertionHandler aperture_get_assertion_handler(void) noexcept {
  if (aperture::GetAssertionHandler() != &CppToCAssertionThunk) {
    return nullptr;
  }
  return g_c_assertion_handler.load(std::memory_order_relaxed);
}
}
