#include "window.hpp"

#include <aperture/assert.hpp>
#include <aperture/log.hpp>
#include <aperture/result.hpp>
#include <aperture/surface.hpp>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include <GLFW/glfw3.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#else
#ifdef APERTURE_EXAMPLES_X11
#define GLFW_EXPOSE_NATIVE_X11
#endif
#ifdef APERTURE_EXAMPLES_WAYLAND
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#endif

#include <GLFW/glfw3native.h>

#include <algorithm>
#include <cstdint>
#include <expected>

#ifdef _WIN32
#ifdef CreateWindow
#undef CreateWindow
#endif
#endif

namespace aperture::examples {

namespace {

uint32_t g_glfw_users = 0;

void GlfwError(int error, const char* description) {
  log::Error("GLFW error {}: {}!", error,
             description != nullptr ? description : "");
}

[[nodiscard]] bool RetainGlfw() noexcept {
  if (g_glfw_users == 0) {
    glfwSetErrorCallback(GlfwError);
    if (glfwInit() != GLFW_TRUE) {
      glfwSetErrorCallback(nullptr);
      return false;
    }
  }
  ++g_glfw_users;
  return true;
}

void ReleaseGlfw() noexcept {
  APERTURE_ASSERT(g_glfw_users > 0);
  --g_glfw_users;
  if (g_glfw_users == 0) {
    glfwTerminate();
    glfwSetErrorCallback(nullptr);
  }
}

[[nodiscard]] constexpr uint2 ToUint2(int width, int height) noexcept {
  return {static_cast<uint32_t>(std::max(width, 0)),
          static_cast<uint32_t>(std::max(height, 0))};
}

}  // namespace

auto CreateWindow(const WindowDesc& desc) noexcept -> Result<Window> {
  if (!RetainGlfw()) [[unlikely]] {
    return std::unexpected(Error::Unsupported);
  }

  glfwDefaultWindowHints();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, desc.resizable ? GLFW_TRUE : GLFW_FALSE);
  glfwWindowHint(GLFW_VISIBLE, desc.visible ? GLFW_TRUE : GLFW_FALSE);

  GLFWwindow* glfw_window = glfwCreateWindow(
      static_cast<int>(desc.width), static_cast<int>(desc.height),
      desc.title.c_str(), nullptr, nullptr);
  if (glfw_window == nullptr) [[unlikely]] {
    ReleaseGlfw();
    return std::unexpected(Error::Invalid);
  }
  return Window{.ptr = glfw_window};
}

void Destroy(Window window) noexcept {
  if (!window) {
    return;
  }
  glfwDestroyWindow(window.ptr);
  ReleaseGlfw();
}

void PollWindowEvents() noexcept {
  glfwPollEvents();
}

void Close(Window window) noexcept {
  APERTURE_ASSERT(window.ptr != nullptr);
  glfwSetWindowShouldClose(window.ptr, GLFW_TRUE);
}

bool ShouldClose(Window window) noexcept {
  APERTURE_ASSERT(window.ptr != nullptr);
  return glfwWindowShouldClose(window.ptr) == GLFW_TRUE;
}

uint2 Size(Window window) noexcept {
  APERTURE_ASSERT(window.ptr != nullptr);
  int width = 0;
  int height = 0;
  glfwGetWindowSize(window.ptr, &width, &height);
  return ToUint2(width, height);
}

uint2 FramebufferSize(Window window) noexcept {
  APERTURE_ASSERT(window.ptr != nullptr);
  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(window.ptr, &width, &height);
  return ToUint2(width, height);
}

auto Surface(Window window) noexcept -> Result<aperture::Surface> {
  APERTURE_ASSERT(window.ptr != nullptr);

#ifdef _WIN32
  return aperture::Surface{
      .platform_surface =
          Win32Surface{
              .hwnd = glfwGetWin32Window(window.ptr),
              .hinstance = GetModuleHandleW(nullptr),
          },
  };
#elif defined(__APPLE__)
  return aperture::Surface{
      .platform_surface = CocoaSurface{.layer = glfwGetCocoaWindow(window.ptr)},
  };
#else
  switch (glfwGetPlatform()) {
#ifdef GLFW_EXPOSE_NATIVE_WAYLAND
    case GLFW_PLATFORM_WAYLAND:
      return aperture::Surface{
          .platform_surface =
              WaylandSurface{
                  .display = glfwGetWaylandDisplay(),
                  .surface = glfwGetWaylandWindow(window.ptr),
              },
      };
#endif
#ifdef GLFW_EXPOSE_NATIVE_X11
    case GLFW_PLATFORM_X11:
      return aperture::Surface{
          .platform_surface =
              XlibSurface{
                  .display = glfwGetX11Display(),
                  .window = glfwGetX11Window(window.ptr),
              },
      };
#endif
    default:
      log::Error("GLFW platform has no aperture surface mapping ({})!",
                 ToString(Error::Unsupported));
      return std::unexpected(Error::Unsupported);
  }
#endif
}

}  // namespace aperture::examples
