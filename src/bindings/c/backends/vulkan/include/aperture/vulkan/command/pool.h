#ifndef APERTURE_VULKAN_COMMAND_POOL_H
#define APERTURE_VULKAN_COMMAND_POOL_H

#include <aperture/command/pool.h>
#include <aperture/platform.h>
#include <aperture/vulkan/header.h>

APERTURE_C_BEGIN

/// @brief Live `VkCommandPool` for a portable command pool.
/// @param pool Portable pool
/// @return Native `VkCommandPool`
/// @warning Asserts in next cases:
/// - If `pool` is null
/// - If `pool` is not Vulkan
APERTURE_C_API VkCommandPool aperture_vk_command_pool(ApertureCommandPool pool)
    APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
