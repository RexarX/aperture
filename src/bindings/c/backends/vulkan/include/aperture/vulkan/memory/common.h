#ifndef APERTURE_VULKAN_MEMORY_COMMON_H
#define APERTURE_VULKAN_MEMORY_COMMON_H

#include <aperture/device.h>
#include <aperture/platform.h>
#include <aperture/types.h>
#include <aperture/vulkan/header.h>

#include <stdint.h>

APERTURE_C_BEGIN

/// @brief Spanning `VkBuffer` plus a byte range.
typedef struct ApertureVkBuffer {
  VkBuffer buffer;
  uint64_t address;
  uint64_t offset;
  uint64_t size;
} ApertureVkBuffer;

/// @brief Spanning buffer that contains `ptr`.
/// @param device Device that owns the allocation
/// @param ptr Device address inside a live allocation
/// @return Spanning `VkBuffer` plus byte range
/// @warning Asserts if `device` is null or `ptr` is unknown.
APERTURE_C_API ApertureVkBuffer aperture_vk_buffer(
    ApertureDevice device, ApertureGpuPtr ptr) APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
