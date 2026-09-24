#ifndef APERTURE_VULKAN_DESCRIPTOR_H
#define APERTURE_VULKAN_DESCRIPTOR_H

#include <aperture/platform.h>

#include <stdint.h>

APERTURE_C_BEGIN

/// @brief Maximum size of a Vulkan descriptor blob, in bytes.
#define APERTURE_VK_MAX_DESCRIPTOR_BYTES 64U

/// @brief Opaque descriptor bytes for a user-managed Vulkan heap.
typedef struct ApertureVkDescriptor {
  APERTURE_C_ALIGNAS(8) uint8_t data[APERTURE_VK_MAX_DESCRIPTOR_BYTES];
  uint32_t size;
} ApertureVkDescriptor;

APERTURE_C_END

#endif
