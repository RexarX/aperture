#ifndef APERTURE_VULKAN_SURFACE_H
#define APERTURE_VULKAN_SURFACE_H

#include <aperture/instance.h>
#include <aperture/platform.h>
#include <aperture/result.h>
#include <aperture/surface.h>
#include <aperture/vulkan/header.h>

APERTURE_C_BEGIN

/// @brief Vulkan WSI surface.
typedef struct ApertureVkSurface {
  VkSurfaceKHR surface;
} ApertureVkSurface;

/// @brief Creates a `VkSurfaceKHR` from an OS window handle.
/// @param instance Instance that owns the surface
/// @param surface OS window handle
/// @param out Receives the Vulkan surface on success
/// @return The surface error, or `APERTURE_ERROR_OK`
/// @warning Asserts if `instance` or `surface` is null.
APERTURE_C_API ApertureError aperture_vk_create_surface(
    ApertureInstance instance, const ApertureSurface* surface,
    ApertureVkSurface* out) APERTURE_C_NOEXCEPT;

/// @brief Destroys a `VkSurfaceKHR`. Null `surface` is a no-op.
/// @param instance Instance that owns the surface
/// @param surface Vulkan surface to destroy
/// @warning Asserts if `instance` is null.
APERTURE_C_API void aperture_vk_destroy_surface(
    ApertureInstance instance, ApertureVkSurface surface) APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
