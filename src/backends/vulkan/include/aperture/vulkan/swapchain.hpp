#pragma once

#include <aperture/swapchain.hpp>
#include <aperture/vulkan/surface.hpp>

#include <aperture/vulkan/header.hpp>

#include <span>

#ifndef VK_KHR_PRESENT_WAIT_EXTENSION_NAME
#define VK_KHR_PRESENT_WAIT_EXTENSION_NAME "VK_KHR_present_wait"
#endif
#ifndef VK_KHR_PRESENT_ID_EXTENSION_NAME
#define VK_KHR_PRESENT_ID_EXTENSION_NAME "VK_KHR_present_id"
#endif

namespace aperture::vk {

/// @brief One entry in the closed `PresentMode` ↔ `VkPresentModeKHR` table.
/// @details `PresentMode::Waitable` is adapter-level (`VK_KHR_present_wait` /
/// `VK_KHR_present_id`) and is not a `VkPresentModeKHR`.
struct PresentModeEntry {
  VkPresentModeKHR vk = VK_PRESENT_MODE_FIFO_KHR;
  PresentMode mode = PresentMode::None;
};

/// @brief Closed mapping used by `ToVkPresentMode` / `FromVkPresentMode`.
inline constexpr PresentModeEntry APERTURE_TO_VK_PRESENT_MODE[] = {
    {.vk = VK_PRESENT_MODE_FIFO_KHR, .mode = PresentMode::Fifo},
    {.vk = VK_PRESENT_MODE_IMMEDIATE_KHR, .mode = PresentMode::Immediate},
    {.vk = VK_PRESENT_MODE_MAILBOX_KHR, .mode = PresentMode::Mailbox},
};

/// @brief Sentinel returned by `ToVkPresentMode` when `mode` is not a WSI
/// present mode (`None`, `Waitable`, or a combined mask).
inline constexpr auto PRESENT_MODE_INVALID =
    static_cast<VkPresentModeKHR>(0x7FFFFFFF);

/// @brief Vulkan present mode for a single `PresentMode` bit.
/// @param mode Exact enumerator to convert
/// @return Matching `VkPresentModeKHR`, or `PRESENT_MODE_INVALID`
[[nodiscard]] constexpr VkPresentModeKHR ToVkPresentMode(
    PresentMode mode) noexcept {
  for (const PresentModeEntry entry : APERTURE_TO_VK_PRESENT_MODE) {
    if (entry.mode == mode) {
      return entry.vk;
    }
  }
  return PRESENT_MODE_INVALID;
}

/// @brief Portable present-mode bit for a Vulkan present mode.
/// @param mode Mode to convert
/// @return Matching enumerator, or `PresentMode::None`
[[nodiscard]] constexpr PresentMode FromVkPresentMode(
    VkPresentModeKHR mode) noexcept {
  for (const PresentModeEntry entry : APERTURE_TO_VK_PRESENT_MODE) {
    if (entry.vk == mode) {
      return entry.mode;
    }
  }
  return PresentMode::None;
}

/// @brief Portable present-mode mask for a list of Vulkan present modes.
/// @param modes Modes reported for a surface
/// @return Union of mapped bits. `Waitable` is never set here
[[nodiscard]] constexpr PresentMode PresentModesFromVk(
    std::span<const VkPresentModeKHR> modes) noexcept {
  auto mask = PresentMode::None;
  for (const VkPresentModeKHR mode : modes) {
    mask |= FromVkPresentMode(mode);
  }
  return mask;
}

/// @brief Native swapchain bundle. `Destroy` of the aperture object
/// invalidates both handles.
struct Swapchain {
  VkSwapchainKHR swapchain = VK_NULL_HANDLE;
  Surface surface;
};

}  // namespace aperture::vk
