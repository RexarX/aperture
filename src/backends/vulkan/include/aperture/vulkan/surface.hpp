#pragma once

#include <aperture/platform.hpp>
#include <aperture/result.hpp>
#include <aperture/surface.hpp>

#include <aperture/vulkan/header.hpp>

namespace aperture::vk {

struct Instance;

/// @brief Vulkan WSI surface. Destroy with `DestroySurface`.
struct Surface {
  VkSurfaceKHR surface = VK_NULL_HANDLE;
};

/// @brief Creates a `VkSurfaceKHR` from an OS window handle.
/// @param instance Instance that owns the Vulkan instance
/// @param surface Presentation surface to wrap
/// @return The surface, or a recoverable `Error`
/// @warning Asserts if `instance` is null.
[[nodiscard]] APERTURE_API auto CreateSurface(
    Instance* instance, const aperture::Surface& surface) noexcept
    -> Result<Surface>;

/// @brief Destroys a `VkSurfaceKHR`. Null `surface` is a no-op.
/// @param instance Instance that created `surface`
/// @param surface Surface to destroy
/// @warning Asserts if `instance` is null.
APERTURE_API void DestroySurface(Instance* instance, Surface surface) noexcept;

}  // namespace aperture::vk
