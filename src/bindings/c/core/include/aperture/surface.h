#ifndef APERTURE_SURFACE_H
#define APERTURE_SURFACE_H

#include <stdint.h>

/// @brief Which alternative is live in `AperturePlatformSurface`.
typedef uint8_t ApertureSurfaceKind;

enum {
  APERTURE_SURFACE_KIND_WIN32 = 0U,
  APERTURE_SURFACE_KIND_XLIB,
  APERTURE_SURFACE_KIND_XCB,
  APERTURE_SURFACE_KIND_WAYLAND,
  APERTURE_SURFACE_KIND_COCOA,
  APERTURE_SURFACE_KIND_ANDROID,
};

/// @brief Win32 `HWND` / `HINSTANCE` pair.
typedef struct ApertureWin32Surface {
  void* hwnd;
  void* hinstance;
} ApertureWin32Surface;

/// @brief Xlib `Display*` / `Window` pair.
typedef struct ApertureXlibSurface {
  void* display;
  uint64_t window;
} ApertureXlibSurface;

/// @brief XCB connection / window.
typedef struct ApertureXcbSurface {
  void* connection;
  uint32_t window;
} ApertureXcbSurface;

/// @brief Wayland `wl_display*` / `wl_surface*`.
typedef struct ApertureWaylandSurface {
  void* display;
  void* surface;
} ApertureWaylandSurface;

/// @brief Cocoa `CAMetalLayer*`.
typedef struct ApertureCocoaSurface {
  void* layer;
} ApertureCocoaSurface;

/// @brief Android `ANativeWindow*`.
typedef struct ApertureAndroidSurface {
  void* window;
} ApertureAndroidSurface;

/// @brief OS window handle.
typedef struct AperturePlatformSurface {
  ApertureSurfaceKind kind;
  union {
    ApertureWin32Surface win32;
    ApertureXlibSurface xlib;
    ApertureXcbSurface xcb;
    ApertureWaylandSurface wayland;
    ApertureCocoaSurface cocoa;
    ApertureAndroidSurface android;
  } u;
} AperturePlatformSurface;

/// @brief Presentation surface wrapping a platform window handle.
typedef struct ApertureSurface {
  AperturePlatformSurface platform_surface;
} ApertureSurface;

#endif
