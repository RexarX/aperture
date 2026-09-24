#pragma once

#include <aperture/format.hpp>

#include <aperture/vulkan/header.hpp>

namespace aperture::vk {

/// @brief One entry in the closed `TextureFormat` ↔ `VkFormat` table.
struct FormatEntry {
  VkFormat vk = VK_FORMAT_UNDEFINED;
  TextureFormat format = TextureFormat::Undefined;
};

/// @brief Closed mapping used by `ToVkFormat` / `FromVkFormat`.
inline constexpr FormatEntry APERTURE_TO_VK_FORMAT[] = {
    {.vk = VK_FORMAT_R8_UNORM, .format = TextureFormat::R8Unorm},
    {.vk = VK_FORMAT_R8G8_UNORM, .format = TextureFormat::R8G8Unorm},
    {.vk = VK_FORMAT_R8G8B8A8_UNORM, .format = TextureFormat::R8G8B8A8Unorm},
    {.vk = VK_FORMAT_R8G8B8A8_SRGB, .format = TextureFormat::R8G8B8A8Srgb},
    {.vk = VK_FORMAT_B8G8R8A8_UNORM, .format = TextureFormat::B8G8R8A8Unorm},
    {.vk = VK_FORMAT_B8G8R8A8_SRGB, .format = TextureFormat::B8G8R8A8Srgb},
    {.vk = VK_FORMAT_A2B10G10R10_UNORM_PACK32,
     .format = TextureFormat::A2B10G10R10Unorm},
    {.vk = VK_FORMAT_R16G16B16A16_SFLOAT,
     .format = TextureFormat::R16G16B16A16Float},
    {.vk = VK_FORMAT_R32G32B32A32_SFLOAT,
     .format = TextureFormat::R32G32B32A32Float},
    {.vk = VK_FORMAT_B10G11R11_UFLOAT_PACK32,
     .format = TextureFormat::R11G11B10Float},
    {.vk = VK_FORMAT_D16_UNORM, .format = TextureFormat::D16Unorm},
    {.vk = VK_FORMAT_D24_UNORM_S8_UINT,
     .format = TextureFormat::D24UnormS8Uint},
    {.vk = VK_FORMAT_D32_SFLOAT, .format = TextureFormat::D32Float},
    {.vk = VK_FORMAT_D32_SFLOAT_S8_UINT,
     .format = TextureFormat::D32FloatS8Uint},
    {.vk = VK_FORMAT_BC1_RGBA_UNORM_BLOCK, .format = TextureFormat::Bc1Unorm},
    {.vk = VK_FORMAT_BC1_RGBA_SRGB_BLOCK, .format = TextureFormat::Bc1Srgb},
    {.vk = VK_FORMAT_BC3_UNORM_BLOCK, .format = TextureFormat::Bc3Unorm},
    {.vk = VK_FORMAT_BC3_SRGB_BLOCK, .format = TextureFormat::Bc3Srgb},
    {.vk = VK_FORMAT_BC5_UNORM_BLOCK, .format = TextureFormat::Bc5Unorm},
    {.vk = VK_FORMAT_BC7_UNORM_BLOCK, .format = TextureFormat::Bc7Unorm},
    {.vk = VK_FORMAT_BC7_SRGB_BLOCK, .format = TextureFormat::Bc7Srgb},
};

/// @brief Vulkan format for a closed `TextureFormat`.
/// @param format Format to convert
/// @return Matching `VkFormat`, or `VK_FORMAT_UNDEFINED`
[[nodiscard]] constexpr VkFormat ToVkFormat(TextureFormat format) noexcept {
  for (const FormatEntry entry : APERTURE_TO_VK_FORMAT) {
    if (entry.format == format) {
      return entry.vk;
    }
  }
  return VK_FORMAT_UNDEFINED;
}

/// @brief Closed `TextureFormat` for a Vulkan format.
/// @param format Format to convert
/// @return Matching enumerator, or `TextureFormat::Undefined`
[[nodiscard]] constexpr TextureFormat FromVkFormat(VkFormat format) noexcept {
  for (const FormatEntry entry : APERTURE_TO_VK_FORMAT) {
    if (entry.vk == format) {
      return entry.format;
    }
  }
  return TextureFormat::Undefined;
}

}  // namespace aperture::vk
