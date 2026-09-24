#ifndef APERTURE_VULKAN_FORMAT_H
#define APERTURE_VULKAN_FORMAT_H

#include <aperture/format.h>
#include <aperture/vulkan/header.h>

#include <stddef.h>
#include <stdint.h>

/// @brief One entry in the closed `ApertureTextureFormat` <=> `VkFormat` table.
typedef struct ApertureVkFormatEntry {
  VkFormat vk;
  ApertureTextureFormat format;
} ApertureVkFormatEntry;

/// @brief Closed mapping used by `aperture_vk_to_vk_format`.
static const ApertureVkFormatEntry APERTURE_TO_VK_FORMAT[] = {
    {VK_FORMAT_R8_UNORM, APERTURE_TEXTURE_FORMAT_R8_UNORM},
    {VK_FORMAT_R8G8_UNORM, APERTURE_TEXTURE_FORMAT_R8G8_UNORM},
    {VK_FORMAT_R8G8B8A8_UNORM, APERTURE_TEXTURE_FORMAT_R8G8B8A8_UNORM},
    {VK_FORMAT_R8G8B8A8_SRGB, APERTURE_TEXTURE_FORMAT_R8G8B8A8_SRGB},
    {VK_FORMAT_B8G8R8A8_UNORM, APERTURE_TEXTURE_FORMAT_B8G8R8A8_UNORM},
    {VK_FORMAT_B8G8R8A8_SRGB, APERTURE_TEXTURE_FORMAT_B8G8R8A8_SRGB},
    {VK_FORMAT_A2B10G10R10_UNORM_PACK32,
     APERTURE_TEXTURE_FORMAT_A2B10G10R10_UNORM},
    {VK_FORMAT_R16G16B16A16_SFLOAT, APERTURE_TEXTURE_FORMAT_R16G16B16A16_FLOAT},
    {VK_FORMAT_R32G32B32A32_SFLOAT, APERTURE_TEXTURE_FORMAT_R32G32B32A32_FLOAT},
    {VK_FORMAT_B10G11R11_UFLOAT_PACK32,
     APERTURE_TEXTURE_FORMAT_R11G11B10_FLOAT},
    {VK_FORMAT_D16_UNORM, APERTURE_TEXTURE_FORMAT_D16_UNORM},
    {VK_FORMAT_D24_UNORM_S8_UINT, APERTURE_TEXTURE_FORMAT_D24_UNORM_S8_UINT},
    {VK_FORMAT_D32_SFLOAT, APERTURE_TEXTURE_FORMAT_D32_FLOAT},
    {VK_FORMAT_D32_SFLOAT_S8_UINT, APERTURE_TEXTURE_FORMAT_D32_FLOAT_S8_UINT},
    {VK_FORMAT_BC1_RGBA_UNORM_BLOCK, APERTURE_TEXTURE_FORMAT_BC1_UNORM},
    {VK_FORMAT_BC1_RGBA_SRGB_BLOCK, APERTURE_TEXTURE_FORMAT_BC1_SRGB},
    {VK_FORMAT_BC3_UNORM_BLOCK, APERTURE_TEXTURE_FORMAT_BC3_UNORM},
    {VK_FORMAT_BC3_SRGB_BLOCK, APERTURE_TEXTURE_FORMAT_BC3_SRGB},
    {VK_FORMAT_BC5_UNORM_BLOCK, APERTURE_TEXTURE_FORMAT_BC5_UNORM},
    {VK_FORMAT_BC7_UNORM_BLOCK, APERTURE_TEXTURE_FORMAT_BC7_UNORM},
    {VK_FORMAT_BC7_SRGB_BLOCK, APERTURE_TEXTURE_FORMAT_BC7_SRGB},
};

/// @brief Vulkan format for a closed `ApertureTextureFormat`.
/// @param format Closed texture format
/// @return Matching `VkFormat`, or `VK_FORMAT_UNDEFINED`
static inline VkFormat aperture_vk_to_vk_format(ApertureTextureFormat format) {
  const size_t count =
      sizeof(APERTURE_TO_VK_FORMAT) / sizeof(APERTURE_TO_VK_FORMAT[0]);
  for (size_t i = 0; i < count; ++i) {
    if (APERTURE_TO_VK_FORMAT[i].format == format) {
      return APERTURE_TO_VK_FORMAT[i].vk;
    }
  }
  return VK_FORMAT_UNDEFINED;
}

/// @brief Closed `ApertureTextureFormat` for a Vulkan format.
/// @param format Vulkan format
/// @return Matching closed format, or `APERTURE_TEXTURE_FORMAT_UNDEFINED`
static inline ApertureTextureFormat aperture_vk_from_vk_format(
    VkFormat format) {
  const size_t count =
      sizeof(APERTURE_TO_VK_FORMAT) / sizeof(APERTURE_TO_VK_FORMAT[0]);
  for (size_t i = 0; i < count; ++i) {
    if (APERTURE_TO_VK_FORMAT[i].vk == format) {
      return APERTURE_TO_VK_FORMAT[i].format;
    }
  }
  return APERTURE_TEXTURE_FORMAT_UNDEFINED;
}

#endif
