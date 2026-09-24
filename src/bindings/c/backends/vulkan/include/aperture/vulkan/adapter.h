#ifndef APERTURE_VULKAN_ADAPTER_H
#define APERTURE_VULKAN_ADAPTER_H

#include <aperture/adapter.h>
#include <aperture/platform.h>
#include <aperture/vulkan/header.h>

#include <stdbool.h>
#include <stddef.h>

APERTURE_C_BEGIN

/// @brief `VkPhysicalDevice` of a portable adapter.
/// @param adapter Adapter to query
/// @return Native `VkPhysicalDevice`
/// @warning Asserts if `adapter` is null or not Vulkan.
APERTURE_C_API VkPhysicalDevice aperture_vk_adapter_physical_device(
    const ApertureAdapter* adapter) APERTURE_C_NOEXCEPT;

/// @brief True if `name` is in this adapter's device extensions.
/// @param adapter Adapter to query
/// @param name Extension name
/// @return `true` if the adapter enumerated `name`
/// @warning Asserts if `adapter` or `name` is null.
APERTURE_C_API bool aperture_vk_has_device_extension(
    const ApertureAdapter* adapter, const char* name) APERTURE_C_NOEXCEPT;

/// @brief Device extensions of this adapter.
/// @param adapter Adapter to query
/// @param data Receives pointer to the extension array
/// @param size Receives element count
/// @warning Asserts if `adapter`, `data`, or `size` is null.
APERTURE_C_API void aperture_vk_device_extensions(
    const ApertureAdapter* adapter, const VkExtensionProperties** data,
    size_t* size) APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
