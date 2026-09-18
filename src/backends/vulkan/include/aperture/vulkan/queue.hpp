#pragma once

#include <aperture/platform.hpp>
#include <aperture/queue.hpp>
#include <aperture/result.hpp>

#include <aperture/vulkan/header.hpp>

namespace aperture::vk {

struct Device;

/// @brief Vulkan queue object owned by a `Device`.
struct Queue {
  Device* device = nullptr;
  VkQueue queue = VK_NULL_HANDLE;
  uint32_t family = 0;
  QueueUsage usage = QueueUsage::Graphics;
};

/// @brief Graphics queue of a Vulkan device. Always present.
/// @param device Device that owns the queue
/// @return Graphics queue
/// @warning Asserts if `device` is null.
[[nodiscard]] APERTURE_API Queue* GraphicsQueue(Device* device) noexcept;

/// @brief Dedicated compute queue. Requires `Capability::AsyncCompute`.
/// @param device Device that owns the queue
/// @return The queue, or `Error::Unsupported` if the device has none
/// @warning Asserts if `device` is null.
[[nodiscard]] APERTURE_API auto ComputeQueue(Device* device) noexcept
    -> Result<Queue*>;

/// @brief Dedicated copy queue. Requires `Capability::AsyncCopy`.
/// @param device Device that owns the queue
/// @return The queue, or `Error::Unsupported` if the device has none
/// @warning Asserts if `device` is null.
[[nodiscard]] APERTURE_API auto CopyQueue(Device* device) noexcept
    -> Result<Queue*>;

/// @brief Live Vulkan queue object for `queue`.
/// @param queue Portable queue handle
/// @return Reference valid until `Destroy` of the owning device
/// @warning Asserts if `queue` is null or not a Vulkan queue.
[[nodiscard]] APERTURE_API Queue& GetNative(aperture::Queue queue) noexcept;

}  // namespace aperture::vk
