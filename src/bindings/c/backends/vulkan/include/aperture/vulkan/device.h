#ifndef APERTURE_VULKAN_DEVICE_H
#define APERTURE_VULKAN_DEVICE_H

#include <aperture/device.h>
#include <aperture/instance.h>
#include <aperture/platform.h>
#include <aperture/result.h>
#include <aperture/vulkan/header.h>

#include <stddef.h>

APERTURE_C_BEGIN

/// @brief Additive device extensions and a `pNext` feature chain.
typedef struct ApertureVkDeviceExtras {
  const char* const* extensions;
  const VkPhysicalDeviceFeatures2* features;
  size_t extension_count;
} ApertureVkDeviceExtras;

/// @brief Creates a Vulkan device. `extras` may be `NULL` (empty extras).
/// @param instance Instance that owns `desc->adapter`
/// @param desc Device creation parameters
/// @param extras Extra extensions and feature chain, or `NULL`
/// @param out Receives the device on success
/// @return The device error, or `APERTURE_ERROR_OK`
/// @warning Asserts if `instance`, `desc`, or `out` is null.
APERTURE_C_API ApertureError aperture_vk_create_device(
    ApertureInstance instance, const ApertureDeviceDesc* desc,
    const ApertureVkDeviceExtras* extras,
    ApertureDevice* out) APERTURE_C_NOEXCEPT;

/// @brief Live `VkDevice` for a portable device.
/// @param device Portable device
/// @return Native `VkDevice`
/// @warning Asserts if `device` is null or not Vulkan.
APERTURE_C_API VkDevice aperture_vk_device(ApertureDevice device)
    APERTURE_C_NOEXCEPT;

/// @brief Live `VkPhysicalDevice` for a portable device.
/// @param device Portable device
/// @return Native `VkPhysicalDevice`
/// @warning Asserts if `device` is null or not Vulkan.
APERTURE_C_API VkPhysicalDevice
aperture_vk_physical_device(ApertureDevice device) APERTURE_C_NOEXCEPT;

/// @brief Live `VkInstance` that owns `device`.
/// @param device Portable device
/// @return Native `VkInstance`
/// @warning Asserts if `device` is null or not Vulkan.
APERTURE_C_API VkInstance aperture_vk_device_instance(ApertureDevice device)
    APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
