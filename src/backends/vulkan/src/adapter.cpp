#include <pch.hpp>

#include <aperture/vulkan/adapter.hpp>

#include <aperture/assert.hpp>
#include <aperture/format.hpp>
#include <aperture/platform.hpp>
#include <aperture/surface.hpp>
#include <aperture/swapchain.hpp>
#include "internal.hpp"

#include <aperture/vulkan/format.hpp>
#include <aperture/vulkan/instance.hpp>
#include <aperture/vulkan/surface.hpp>
#include <aperture/vulkan/swapchain.hpp>

#include <aperture/vulkan/header.hpp>

#include <algorithm>
#include <cstdint>
#include <memory_resource>
#include <span>
#include <string_view>

namespace aperture::vk {

namespace {

[[nodiscard]] constexpr VkFormatFeatureFlags UsageMask(
    FormatUsage usage) noexcept {
  VkFormatFeatureFlags flags = 0;
  if (HasAll(usage, FormatUsage::Sample)) {
    flags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
  }
  if (HasAll(usage, FormatUsage::Filter)) {
    flags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
  }
  if (HasAll(usage, FormatUsage::Storage)) {
    flags |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
  }
  if (HasAll(usage, FormatUsage::Color)) {
    flags |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
  }
  if (HasAll(usage, FormatUsage::Depth)) {
    flags |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }
  if (HasAll(usage, FormatUsage::Blend)) {
    flags |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;
  }
  if (HasAll(usage, FormatUsage::Copy)) {
    flags |=
        VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
  }
  if (HasAll(usage, FormatUsage::Resolve)) {
    flags |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
  }
  if (HasAll(usage, FormatUsage::Atomic)) {
    flags |= VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT;
  }
  return flags;
}

[[nodiscard]] bool FormatSupports(VkPhysicalDevice physical,
                                  TextureFormat format,
                                  FormatUsage usage) noexcept {
  const VkFormat vk_format = ToVkFormat(format);
  if (vk_format == VK_FORMAT_UNDEFINED) {
    return false;
  }
  VkFormatProperties2 props{.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
  vkGetPhysicalDeviceFormatProperties2(physical, vk_format, &props);
  const VkFormatFeatureFlags needed = UsageMask(usage);
  return (props.formatProperties.optimalTilingFeatures & needed) == needed;
}

}  // namespace

bool SupportsFormat(const Adapter& adapter, TextureFormat format,
                    FormatUsage usage) noexcept {
  if (adapter.physical_device == VK_NULL_HANDLE) {
    return false;
  }
  return FormatSupports(adapter.physical_device, format, usage);
}

auto PresentableFormats(Adapter* adapter,
                        const aperture::Surface& surface) noexcept
    -> std::span<const TextureFormat> {
  if (adapter == nullptr || adapter->owner == nullptr) {
    return {};
  }
  adapter->presentable_format_count = 0;
  auto created = CreateSurface(adapter->owner, surface);
  if (!created) {
    return {};
  }
  const Surface vk_surface = *created;
  const VkSurfaceKHR handle = vk_surface.surface;

  Scratch scratch;
  std::pmr::vector<VkSurfaceFormatKHR> formats{&scratch.resource};
  std::pmr::vector<VkPresentModeKHR> modes{&scratch.resource};
  const auto formats_ok = EnumerateVk(
      &formats,
      [adapter, handle](uint32_t* count, VkSurfaceFormatKHR* data) noexcept {
        return vkGetPhysicalDeviceSurfaceFormatsKHR(adapter->physical_device,
                                                    handle, count, data);
      });
  const auto modes_ok = EnumerateVk(
      &modes,
      [adapter, handle](uint32_t* count, VkPresentModeKHR* data) noexcept {
        return vkGetPhysicalDeviceSurfacePresentModesKHR(
            adapter->physical_device, handle, count, data);
      });
  DestroySurface(adapter->owner, vk_surface);
  if (!formats_ok || !modes_ok) {
    return {};
  }

  PresentMode mask = PresentModesFromVk(modes);
  if (HasDeviceExtension(*adapter, VK_KHR_PRESENT_WAIT_EXTENSION_NAME) &&
      HasDeviceExtension(*adapter, VK_KHR_PRESENT_ID_EXTENSION_NAME)) {
    mask |= PresentMode::Waitable;
  }

  if (mask != PresentMode::None &&
      adapter->index < adapter->owner->adapters.size()) {
    adapter->owner->adapters[adapter->index].present_modes = mask;
  }

  for (const VkSurfaceFormatKHR& fmt : formats) {
    const TextureFormat mapped = FromVkFormat(fmt.format);
    if (mapped == TextureFormat::Undefined) {
      continue;
    }

    const auto begin = adapter->presentable_formats.begin();
    const auto end = begin + adapter->presentable_format_count;
    if (std::find(begin, end, mapped) != end) {
      continue;
    }

    if (adapter->presentable_format_count >= PRESENTABLE_FORMAT_CAPACITY) {
      break;
    }
    adapter->presentable_formats[adapter->presentable_format_count++] = mapped;
  }
  return {adapter->presentable_formats.data(),
          adapter->presentable_format_count};
}

bool HasDeviceExtension(const Adapter& adapter,
                        std::string_view name) noexcept {
  return HasExtension(DeviceExtensions(adapter), name);
}

Adapter& GetNative(const aperture::Adapter& adapter) noexcept {
  APERTURE_ASSERT(adapter.impl != nullptr);
  auto* record = static_cast<Adapter*>(adapter.impl);
  APERTURE_ASSERT(record->backend == Backend::Vulkan);
  return *record;
}

}  // namespace aperture::vk
