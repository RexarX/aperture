#ifndef APERTURE_VULKAN_COMMAND_BUFFER_H
#define APERTURE_VULKAN_COMMAND_BUFFER_H

#include <aperture/command/buffer.h>
#include <aperture/platform.h>
#include <aperture/vulkan/header.h>

APERTURE_C_BEGIN

/// @brief Live `VkCommandBuffer` for a portable recording token.
/// @param buffer Portable recording token
/// @return Native `VkCommandBuffer`
/// @warning Asserts in next cases:
/// - If `buffer` is null
/// - If `buffer` is not Vulkan
APERTURE_C_API VkCommandBuffer
aperture_vk_command_buffer(ApertureCommandBuffer buffer) APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
