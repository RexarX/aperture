#include "window.h"

#include <aperture/log.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
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

#include <stddef.h>

static uint32_t g_glfw_users = 0;

static void GlfwError(int error, const char* description) {
  aperture_log_errorf("GLFW error %d: %s!", error,
                      description != NULL ? description : "");
}

static bool RetainGlfw(void) {
  if (g_glfw_users == 0) {
    glfwSetErrorCallback(GlfwError);
    if (glfwInit() != GLFW_TRUE) {
      glfwSetErrorCallback(NULL);
      return false;
    }
  }
  ++g_glfw_users;
  return true;
}

static void ReleaseGlfw(void) {
  if (g_glfw_users == 0) {
    return;
  }
  --g_glfw_users;
  if (g_glfw_users == 0) {
    glfwTerminate();
    glfwSetErrorCallback(NULL);
  }
}

ApertureError aperture_examples_c_create_window(
    const ApertureExamplesCWindowDesc* desc, ApertureExamplesCWindow* out) {
  if (desc == NULL || out == NULL) {
    return APERTURE_ERROR_INVALID;
  }
  out->ptr = NULL;

  if (!RetainGlfw()) {
    return APERTURE_ERROR_UNSUPPORTED;
  }

  glfwDefaultWindowHints();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, desc->resizable ? GLFW_TRUE : GLFW_FALSE);
  glfwWindowHint(GLFW_VISIBLE, desc->visible ? GLFW_TRUE : GLFW_FALSE);

  const char* title = desc->title != NULL ? desc->title : "aperture";
  GLFWwindow* glfw_window =
      glfwCreateWindow((int)desc->width, (int)desc->height, title, NULL, NULL);
  if (glfw_window == NULL) {
    ReleaseGlfw();
    return APERTURE_ERROR_INVALID;
  }

  out->ptr = glfw_window;
  return APERTURE_ERROR_OK;
}

void aperture_examples_c_destroy_window(ApertureExamplesCWindow window) {
  if (window.ptr == NULL) {
    return;
  }
  glfwDestroyWindow(window.ptr);
  ReleaseGlfw();
}

ApertureError aperture_examples_c_window_surface(ApertureExamplesCWindow window,
                                                 ApertureSurface* out) {
  if (window.ptr == NULL || out == NULL) {
    return APERTURE_ERROR_INVALID;
  }

  ApertureSurface surface = {0};

#ifdef _WIN32
  surface.platform_surface.kind = APERTURE_SURFACE_KIND_WIN32;
  surface.platform_surface.u.win32.hwnd = glfwGetWin32Window(window.ptr);
  surface.platform_surface.u.win32.hinstance = GetModuleHandleW(NULL);
#elif defined(__APPLE__)
  surface.platform_surface.kind = APERTURE_SURFACE_KIND_COCOA;
  surface.platform_surface.u.cocoa.layer = glfwGetCocoaWindow(window.ptr);
#else
  switch (glfwGetPlatform()) {
#ifdef GLFW_EXPOSE_NATIVE_WAYLAND
    case GLFW_PLATFORM_WAYLAND:
      surface.platform_surface.kind = APERTURE_SURFACE_KIND_WAYLAND;
      surface.platform_surface.u.wayland.display = glfwGetWaylandDisplay();
      surface.platform_surface.u.wayland.surface =
          glfwGetWaylandWindow(window.ptr);
      break;
#endif
#ifdef GLFW_EXPOSE_NATIVE_X11
    case GLFW_PLATFORM_X11:
      surface.platform_surface.kind = APERTURE_SURFACE_KIND_XLIB;
      surface.platform_surface.u.xlib.display = glfwGetX11Display();
      surface.platform_surface.u.xlib.window = glfwGetX11Window(window.ptr);
      break;
#endif
    default:
      aperture_log(APERTURE_LOG_LEVEL_ERROR,
                   "GLFW platform has no aperture surface mapping!");
      return APERTURE_ERROR_UNSUPPORTED;
  }
#endif

  *out = surface;
  return APERTURE_ERROR_OK;
}

void aperture_examples_c_poll_window_events(void) {
  glfwPollEvents();
}

void aperture_examples_c_window_size(ApertureExamplesCWindow window,
                                     uint32_t* width_out,
                                     uint32_t* height_out) {
  if (window.ptr == NULL || width_out == NULL || height_out == NULL) {
    return;
  }
  int width = 0;
  int height = 0;
  glfwGetWindowSize(window.ptr, &width, &height);
  *width_out = (uint32_t)width;
  *height_out = (uint32_t)height;
}

void aperture_examples_c_window_framebuffer_size(ApertureExamplesCWindow window,
                                                 uint32_t* width_out,
                                                 uint32_t* height_out) {
  if (window.ptr == NULL || width_out == NULL || height_out == NULL) {
    return;
  }
  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(window.ptr, &width, &height);
  *width_out = (uint32_t)width;
  *height_out = (uint32_t)height;
}
