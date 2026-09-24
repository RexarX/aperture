#ifndef APERTURE_EXAMPLES_C_WINDOW_H
#define APERTURE_EXAMPLES_C_WINDOW_H

#include <aperture/result.h>
#include <aperture/surface.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct GLFWwindow;

/// @brief Parameters for `aperture_examples_c_create_window`.
typedef struct ApertureExamplesCWindowDesc {
  const char* title;
  uint32_t width;
  uint32_t height;
  bool resizable;
  bool visible;

} ApertureExamplesCWindowDesc;

/// @brief OS window used by C examples. Pointer-sized GLFW handle.
typedef struct ApertureExamplesCWindow {
  struct GLFWwindow* ptr;
} ApertureExamplesCWindow;

/// @brief Creates a window with `GLFW_NO_API` (no graphics context).
/// @param desc Window creation parameters
/// @param out Receives the window on success
/// @return The window error, or `APERTURE_ERROR_OK`
ApertureError aperture_examples_c_create_window(
    const ApertureExamplesCWindowDesc* desc, ApertureExamplesCWindow* out);

/// @brief Destroys a window. Null is a no-op.
/// @param window Window to destroy
void aperture_examples_c_destroy_window(ApertureExamplesCWindow window);

/// @brief OS handle for `aperture_presentable_formats` / Vulkan WSI.
/// @param window Live window
/// @param out Receives the presentation surface on success
/// @return The surface error, or `APERTURE_ERROR_OK`
ApertureError aperture_examples_c_window_surface(ApertureExamplesCWindow window,
                                                 ApertureSurface* out);

/// @brief Dispatches pending window events for all windows.
void aperture_examples_c_poll_window_events(void);

/// @brief Client-area size in screen coordinates.
/// @param window Live window
/// @param width_out Receives width in pixels
/// @param height_out Receives height in pixels
void aperture_examples_c_window_size(ApertureExamplesCWindow window,
                                     uint32_t* width_out, uint32_t* height_out);

/// @brief Framebuffer size in pixels.
/// @param window Live window
/// @param width_out Receives width in pixels
/// @param height_out Receives height in pixels
void aperture_examples_c_window_framebuffer_size(ApertureExamplesCWindow window,
                                                 uint32_t* width_out,
                                                 uint32_t* height_out);

#ifdef __cplusplus
}
#endif

#endif
