#ifndef APERTURE_VULKAN_SYNC_H
#define APERTURE_VULKAN_SYNC_H

#include <aperture/platform.h>
#include <aperture/sync.h>
#include <aperture/vulkan/header.h>

APERTURE_C_BEGIN

/// @brief Live timeline `VkSemaphore` for a portable timeline.
/// @param timeline Portable timeline
/// @return Native `VkSemaphore`
/// @warning Asserts in next cases:
/// - If `timeline` is null
/// - If `timeline` is not Vulkan
APERTURE_C_API VkSemaphore aperture_vk_timeline(ApertureTimeline timeline)
    APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
