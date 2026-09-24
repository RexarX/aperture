#ifndef APERTURE_VULKAN_QUEUE_H
#define APERTURE_VULKAN_QUEUE_H

#include <aperture/platform.h>
#include <aperture/queue.h>
#include <aperture/vulkan/header.h>

#include <stdint.h>

APERTURE_C_BEGIN

/// @brief Live `VkQueue` for a portable queue.
/// @param queue Portable queue
/// @return Native `VkQueue`
/// @warning Asserts if `queue` is null or not Vulkan.
APERTURE_C_API VkQueue aperture_vk_queue(ApertureQueue queue)
    APERTURE_C_NOEXCEPT;

/// @brief Queue family index of a portable queue.
/// @param queue Portable queue
/// @return Vulkan queue family index
/// @warning Asserts if `queue` is null or not Vulkan.
APERTURE_C_API uint32_t aperture_vk_queue_family(ApertureQueue queue)
    APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
