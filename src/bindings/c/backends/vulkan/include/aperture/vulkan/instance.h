#ifndef APERTURE_VULKAN_INSTANCE_H
#define APERTURE_VULKAN_INSTANCE_H

#include <aperture/instance.h>
#include <aperture/platform.h>
#include <aperture/result.h>
#include <aperture/vulkan/header.h>

#include <stdbool.h>
#include <stddef.h>

APERTURE_C_BEGIN

/// @brief Additive instance layers and extensions.
typedef struct ApertureVkInstanceExtras {
  const char* const* layers;
  const char* const* extensions;
  size_t layer_count;
  size_t extension_count;
} ApertureVkInstanceExtras;

/// @brief Creates a Vulkan instance. `extras` may be `NULL` (empty extras).
/// @param desc Instance creation parameters
/// @param extras Extra layers and extensions, or `NULL`
/// @param out Receives a portable instance handle on success
/// @return The instance error, or `APERTURE_ERROR_OK`
/// @warning Asserts if `desc` or `out` is null.
APERTURE_C_API ApertureError aperture_vk_create_instance(
    const ApertureInstanceDesc* desc, const ApertureVkInstanceExtras* extras,
    ApertureInstance* out) APERTURE_C_NOEXCEPT;

/// @brief Live `VkInstance` for a portable instance.
/// @param instance Portable instance
/// @return Native `VkInstance`
/// @warning Asserts if `instance` is null or not Vulkan.
APERTURE_C_API VkInstance aperture_vk_instance(ApertureInstance instance)
    APERTURE_C_NOEXCEPT;

/// @brief Instance extensions enumerated by the loader.
/// @param data Receives pointer to the extension array
/// @param size Receives element count
/// @warning Asserts if `data` or `size` is null.
APERTURE_C_API void aperture_vk_instance_extensions(
    const VkExtensionProperties** data, size_t* size) APERTURE_C_NOEXCEPT;

/// @brief Instance layers enumerated by the loader.
/// @param data Receives pointer to the layer array
/// @param size Receives element count
/// @warning Asserts if `data` or `size` is null.
APERTURE_C_API void aperture_vk_instance_layers(
    const VkLayerProperties** data, size_t* size) APERTURE_C_NOEXCEPT;

/// @brief True if `name` is in `aperture_vk_instance_extensions()`.
/// @param name Extension name
/// @return `true` if the loader enumerated `name`
/// @warning Asserts if `name` is null.
APERTURE_C_API bool aperture_vk_has_instance_extension(const char* name)
    APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
