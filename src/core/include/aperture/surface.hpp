#pragma once

#include <cstdint>
#include <variant>

namespace aperture {

/// @brief Win32 `HWND` / `HINSTANCE` pair for `VK_KHR_win32_surface`.
struct Win32Surface {
  void* hwnd = nullptr;
  void* hinstance = nullptr;
};

/// @brief Xlib `Display*` / `Window` pair for `VK_KHR_xlib_surface`.
struct XlibSurface {
  void* display = nullptr;
  unsigned long window = 0;
};

/// @brief XCB connection / window for `VK_KHR_xcb_surface`.
struct XcbSurface {
  void* connection = nullptr;
  uint32_t window = 0;
};

/// @brief Wayland `wl_display*` / `wl_surface*` for `VK_KHR_wayland_surface`.
struct WaylandSurface {
  void* display = nullptr;
  void* surface = nullptr;
};

/// @brief Cocoa `CAMetalLayer*` (Metal / MoltenVK).
struct CocoaSurface {
  void* layer = nullptr;
};

/// @brief Android `ANativeWindow*` for `VK_KHR_android_surface`.
struct AndroidSurface {
  void* window = nullptr;
};

/// @brief OS window handle. One alternative is live; the rest are unread.
using PlatformSurface =
    std::variant<Win32Surface, XlibSurface, XcbSurface, WaylandSurface,
                 CocoaSurface, AndroidSurface>;

/// @brief Presentation surface wrapping a `PlatformSurface`.
/// @details This is an OS object, not a GPU-API one. `CreateSwapchain` stays
/// backend-generic.
struct Surface {
  PlatformSurface platform_surface;
};

}  // namespace aperture
