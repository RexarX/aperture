#ifndef APERTURE_VULKAN_MEMORY_FREE_H
#define APERTURE_VULKAN_MEMORY_FREE_H

#include <aperture/device.h>
#include <aperture/platform.h>
#include <aperture/types.h>

APERTURE_C_BEGIN

/// @brief Vulkan-backend free of a host-mapped allocation.
/// @param device Device that owns the allocation
/// @param ptr Host-mapped allocation to free
/// @warning Asserts if `device` is null when `ptr` is non-null.
APERTURE_C_API void aperture_vk_free(ApertureDevice device,
                                     ApertureDualPtr ptr) APERTURE_C_NOEXCEPT;

/// @brief Vulkan-backend free of a device-only allocation.
/// @param device Device that owns the allocation
/// @param ptr Device address to free
/// @warning Asserts if `device` is null when `ptr` is non-null.
APERTURE_C_API void aperture_vk_free_gpu(
    ApertureDevice device, ApertureGpuPtr ptr) APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
