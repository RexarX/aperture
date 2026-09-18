#pragma once

#include <aperture/format.hpp>

#include <aperture/vulkan/header.hpp>

namespace aperture::vk {

/// @brief One entry in the closed `TextureFormat` ↔ `VkFormat` table.
struct FormatEntry {
  TextureFormat format = TextureFormat::Undefined;
  VkFormat vk = VK_FORMAT_UNDEFINED;
};

/// @brief Closed mapping used by `ToVkFormat` / `FromVkFormat`.
inline constexpr FormatEntry APERTURE_TO_VK_FORMAT[] = {
    {TextureFormat::R8Unorm, VK_FORMAT_R8_UNORM},
    {TextureFormat::R8G8Unorm, VK_FORMAT_R8G8_UNORM},
    {TextureFormat::R8G8B8A8Unorm, VK_FORMAT_R8G8B8A8_UNORM},
    {TextureFormat::R8G8B8A8Srgb, VK_FORMAT_R8G8B8A8_SRGB},
    {TextureFormat::B8G8R8A8Unorm, VK_FORMAT_B8G8R8A8_UNORM},
    {TextureFormat::B8G8R8A8Srgb, VK_FORMAT_B8G8R8A8_SRGB},
    {TextureFormat::A2B10G10R10Unorm, VK_FORMAT_A2B10G10R10_UNORM_PACK32},
    {TextureFormat::R16G16B16A16Float, VK_FORMAT_R16G16B16A16_SFLOAT},
    {TextureFormat::R32G32B32A32Float, VK_FORMAT_R32G32B32A32_SFLOAT},
    {TextureFormat::R11G11B10Float, VK_FORMAT_B10G11R11_UFLOAT_PACK32},
    {TextureFormat::D16Unorm, VK_FORMAT_D16_UNORM},
    {TextureFormat::D24UnormS8Uint, VK_FORMAT_D24_UNORM_S8_UINT},
    {TextureFormat::D32Float, VK_FORMAT_D32_SFLOAT},
    {TextureFormat::D32FloatS8Uint, VK_FORMAT_D32_SFLOAT_S8_UINT},
    {TextureFormat::Bc1Unorm, VK_FORMAT_BC1_RGBA_UNORM_BLOCK},
    {TextureFormat::Bc1Srgb, VK_FORMAT_BC1_RGBA_SRGB_BLOCK},
    {TextureFormat::Bc3Unorm, VK_FORMAT_BC3_UNORM_BLOCK},
    {TextureFormat::Bc3Srgb, VK_FORMAT_BC3_SRGB_BLOCK},
    {TextureFormat::Bc5Unorm, VK_FORMAT_BC5_UNORM_BLOCK},
    {TextureFormat::Bc7Unorm, VK_FORMAT_BC7_UNORM_BLOCK},
    {TextureFormat::Bc7Srgb, VK_FORMAT_BC7_SRGB_BLOCK},
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
