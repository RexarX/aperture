#pragma once

#include <aperture/assert.hpp>
#include <aperture/capability.hpp>
#include <aperture/device.hpp>
#include <aperture/platform.hpp>
#include <aperture/result.hpp>
#include <aperture/types.hpp>
#include <aperture/vulkan/queue.hpp>

#include <aperture/vulkan/header.hpp>

#include <span>

VK_DEFINE_HANDLE(VmaAllocator)

namespace aperture::vk {

struct Instance;

/// @brief Additive device extensions and a `pNext` feature chain.
struct DeviceExtras {
  std::span<const char* const> extensions;
  const VkPhysicalDeviceFeatures2* features = nullptr;
};

/// @brief Vulkan device object.
/// @details First field is `Backend` so `BackendOf` can read it without
/// knowing the type. Non-copyable: queues point back at this object.
struct Device {
  Backend backend = Backend::Vulkan;
  bool has_compute = false;
  bool has_copy = false;
  bool log_failed_results = true;
  Capability enabled = Capability::None;
  Instance* parent = nullptr;
  VkInstance instance = VK_NULL_HANDLE;
  VkDevice device = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device = VK_NULL_HANDLE;
  VmaAllocator allocator = VK_NULL_HANDLE;
  Queue graphics;
  Queue compute;
  Queue copy;
  DeviceInfo info;
};

/// @brief Creates a Vulkan device. Empty extras equivalent.
/// @param instance Instance that owns `desc.adapter`
/// @param desc Device creation parameters
/// @return The device, or a recoverable `Error`
/// @warning Asserts if `instance` is null.
[[nodiscard]] APERTURE_API auto CreateDevice(Instance* instance,
                                             const DeviceDesc& desc) noexcept
    -> Result<Device*>;

/// @brief Creates a Vulkan device with extra extensions / features.
/// @details Empty extras is equivalent to `vk::CreateDevice(instance, desc)`.
/// @param instance Instance that owns `desc.adapter`
/// @param desc Device creation parameters
/// @param extras Extra extensions and an optional feature chain
/// @return The device, or a recoverable `Error`
/// @warning Asserts if `instance` is null.
[[nodiscard]] APERTURE_API auto CreateDevice(
    Instance* instance, const DeviceDesc& desc,
    const DeviceExtras& extras) noexcept -> Result<Device*>;

/// @brief Destroys a Vulkan device. Immediate; does not wait on the GPU.
/// @param device Device to destroy. Null is a no-op
APERTURE_API void Destroy(Device* device) noexcept;

/// @brief Live Vulkan device object for `device`.
/// @param device Portable device handle
/// @return Reference valid until `Destroy(device)`
/// @warning Asserts if `device` is null or not a Vulkan device.
[[nodiscard]] APERTURE_API Device& GetNative(aperture::Device device) noexcept;

/// @brief Capabilities enabled at `CreateDevice`, not merely reported.
/// @param device Device to query
/// @param caps Required capability bits
/// @return `true` if every bit in `caps` is enabled on `device`
/// @warning Asserts if `device` is null.
[[nodiscard]] APERTURE_API inline bool Has(const Device& device,
                                           Capability caps) noexcept {
  return HasAll(device.enabled, caps);
}

/// @brief Device constants frozen at create time.
/// @param device Device to query
/// @return Reference valid until `Destroy(device)`
/// @warning Asserts if `device` is null.
[[nodiscard]] APERTURE_API inline const DeviceInfo& Info(
    const Device& device) noexcept {
  return device.info;
}

}  // namespace aperture::vk
