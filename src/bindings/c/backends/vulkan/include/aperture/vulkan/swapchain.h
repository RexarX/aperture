#ifndef APERTURE_VULKAN_SWAPCHAIN_H
#define APERTURE_VULKAN_SWAPCHAIN_H

#include <aperture/vulkan/header.h>
#include <aperture/vulkan/surface.h>

/// @brief Native swapchain bundle.
typedef struct ApertureVkSwapchain {
  VkSwapchainKHR swapchain;
  ApertureVkSurface surface;
} ApertureVkSwapchain;

#endif
