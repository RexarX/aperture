#pragma once

#include <aperture/math.hpp>
#include <aperture/result.hpp>
#include <aperture/surface.hpp>

#include <cstdint>
#include <string>

struct GLFWwindow;

namespace aperture::examples {

/// @brief Parameters for `Create`. Copied; the call does not retain
/// references.
struct WindowDesc {
  uint32_t width = 1280;
  uint32_t height = 720;
  bool resizable = true;
  bool visible = true;
  std::string title = "aperture";
};

/// @brief OS window used by examples. Pointer-sized GLFW handle.
struct Window {
  GLFWwindow* ptr = nullptr;

  /// @brief True if this handle is non-null.
  /// @return `true` when `ptr != nullptr`
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return ptr != nullptr;
  }

  [[nodiscard]] constexpr bool operator==(const Window&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const Window&) const noexcept =
      default;
};

/// @brief Creates a window with `GLFW_NO_API` (no graphics context).
/// @param desc Window creation parameters
/// @return The window, or a recoverable `Error`
[[nodiscard]] auto CreateWindow(const WindowDesc& desc) noexcept
    -> Result<Window>;

/// @brief Destroys a window. Null is a no-op.
/// @param window Window to destroy
void Destroy(Window window) noexcept;

/// @brief Dispatches pending window events for all windows.
void PollWindowEvents() noexcept;

/// @brief Requests that the window close.
/// @param window Window to close
/// @warning Asserts if `window` is null.
void Close(Window window) noexcept;

/// @brief True if the user or `Close` requested shutdown.
/// @param window Window to query
/// @return `true` if the window should exit its loop
/// @warning Asserts if `window` is null.
[[nodiscard]] bool ShouldClose(Window window) noexcept;

/// @brief Client-area size in screen coordinates.
/// @param window Window to query
/// @return Width and height in pixels
/// @warning Asserts if `window` is null.
[[nodiscard]] uint2 Size(Window window) noexcept;

/// @brief Framebuffer size in pixels.
/// @param window Window to query
/// @return Width and height of the framebuffer
/// @warning Asserts if `window` is null.
[[nodiscard]] uint2 FramebufferSize(Window window) noexcept;

/// @brief OS handle for `PresentableFormats` / `vk::CreateSurface`.
/// @param window Window to query
/// @return Presentation surface, or `Error::Unsupported`
/// @warning Asserts if `window` is null.
[[nodiscard]] auto Surface(Window window) noexcept -> Result<aperture::Surface>;

}  // namespace aperture::examples
