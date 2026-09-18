#pragma once

#include <aperture/adapter.hpp>
#include <aperture/format.hpp>
#include <aperture/platform.hpp>
#include <aperture/surface.hpp>
#include <aperture/types.hpp>
#include <aperture/vulkan/format.hpp>

#include <aperture/vulkan/header.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace aperture::vk {

struct Instance;

/// @brief Closed-set presentable formats fit in this many slots.
inline constexpr size_t PRESENTABLE_FORMAT_CAPACITY =
    std::size(APERTURE_TO_VK_FORMAT);

/// @brief Per-physical-device Filter record owned by `Instance`.
/// @details First field is `Backend` so `BackendOf` can read it through `impl`.
struct Adapter {
  Backend backend = Backend::Vulkan;
  bool has_descriptor_heap = false;
  uint8_t presentable_format_count = 0;
  uint32_t index = 0;
  Instance* owner = nullptr;
  VkPhysicalDevice physical_device = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties2 properties{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
  VkPhysicalDeviceVulkan11Properties props11{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES};
  VkPhysicalDeviceVulkan12Properties props12{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES};
  VkPhysicalDeviceVulkan13Properties props13{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES};
  VkPhysicalDeviceDescriptorHeapPropertiesEXT descriptor_heap_props{
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_PROPERTIES_EXT};
  VkPhysicalDeviceMemoryProperties memory{};
  VkPhysicalDeviceFeatures2 features2{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
  VkPhysicalDeviceVulkan11Features features11{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
  VkPhysicalDeviceVulkan12Features features12{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
  VkPhysicalDeviceVulkan13Features features13{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
  VkPhysicalDeviceVulkan14Features features14{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES};
  VkPhysicalDeviceRobustness2FeaturesEXT robustness2{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT};
  VkPhysicalDeviceDescriptorHeapFeaturesEXT descriptor_heap_features{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT};
  VkPhysicalDeviceMeshShaderFeaturesEXT mesh_features{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT};
  VkPhysicalDeviceDeviceAddressCommandsFeaturesKHR address_commands{
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_ADDRESS_COMMANDS_FEATURES_KHR};
  VkPhysicalDeviceShaderUntypedPointersFeaturesKHR untyped_pointers{
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_UNTYPED_POINTERS_FEATURES_KHR};
  std::vector<VkQueueFamilyProperties> queue_families;
  std::vector<VkExtensionProperties> extensions;
  std::array<TextureFormat, PRESENTABLE_FORMAT_CAPACITY> presentable_formats =
      {};
};

/// @brief Filter-time format usage query for a Vulkan adapter.
/// @param adapter Adapter to query
/// @param format Format to test
/// @param usage Required usage bits
/// @return `true` if `adapter` supports every bit in `usage` for `format`
[[nodiscard]] APERTURE_API bool SupportsFormat(const Adapter& adapter,
                                               TextureFormat format,
                                               FormatUsage usage) noexcept;

/// @brief Presentable color formats for `surface` on a Vulkan adapter.
/// @param adapter Adapter to query
/// @param surface Presentation surface to present to
/// @return Span valid until the next `PresentableFormats` call on this adapter
/// or `Destroy` of the owning instance
[[nodiscard]] APERTURE_API auto PresentableFormats(
    Adapter* adapter, const aperture::Surface& surface) noexcept
    -> std::span<const TextureFormat>;

/// @brief Live Vulkan adapter record for `adapter`.
/// @param adapter Portable Filter record
/// @return Reference valid until `Destroy` of the owning instance
/// @warning Asserts if `adapter.impl` is null or not a Vulkan adapter.
[[nodiscard]] APERTURE_API Adapter& GetNative(
    const aperture::Adapter& adapter) noexcept;

/// @brief `VkPhysicalDevice` of this adapter.
/// @param adapter Adapter to query
/// @return Native physical device. Valid until `Destroy` of the owning
/// instance
[[nodiscard]] APERTURE_API inline VkPhysicalDevice PhysicalDevice(
    const Adapter& adapter) noexcept {
  return adapter.physical_device;
}

/// @brief `VkPhysicalDeviceProperties2` chain filled at Filter time.
/// @param adapter Adapter to query
/// @return Properties chain. Valid until `Destroy` of the owning instance
[[nodiscard]] APERTURE_API inline const VkPhysicalDeviceProperties2& Properties(
    const Adapter& adapter) noexcept {
  return adapter.properties;
}

/// @brief Memory heaps / types of this adapter.
/// @param adapter Adapter to query
/// @return Memory properties. Valid until `Destroy` of the owning instance
[[nodiscard]] APERTURE_API inline const VkPhysicalDeviceMemoryProperties&
MemoryProperties(const Adapter& adapter) noexcept {
  return adapter.memory;
}

/// @brief Queue families of this adapter.
/// @param adapter Adapter to query
/// @return Span valid until `Destroy` of the owning instance
[[nodiscard]] APERTURE_API inline auto QueueFamilies(
    const Adapter& adapter) noexcept
    -> std::span<const VkQueueFamilyProperties> {
  return adapter.queue_families;
}

/// @brief Device extensions of this adapter.
/// @param adapter Adapter to query
/// @return Span valid until `Destroy` of the owning instance
[[nodiscard]] APERTURE_API inline auto DeviceExtensions(
    const Adapter& adapter) noexcept -> std::span<const VkExtensionProperties> {
  return adapter.extensions;
}

/// @brief True if `name` is in `DeviceExtensions(adapter)`.
/// @param adapter Adapter to query
/// @param name Extension name to look up
/// @return `true` if the device extension is present
[[nodiscard]] APERTURE_API bool HasDeviceExtension(
    const Adapter& adapter, std::string_view name) noexcept;

}  // namespace aperture::vk
