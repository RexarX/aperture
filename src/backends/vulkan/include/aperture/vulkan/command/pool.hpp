#pragma once

#include <aperture/command/pool.hpp>
#include <aperture/platform.hpp>
#include <aperture/queue.hpp>
#include <aperture/result.hpp>
#include <aperture/vulkan/queue.hpp>

#include <aperture/vulkan/header.hpp>

namespace aperture::vk {

struct Device;
struct CommandPoolState;

/// @brief Vulkan command pool. `Backend` is first.
struct CommandPool {
  Backend backend = Backend::Vulkan;
  Device* device = nullptr;
  Queue* queue = nullptr;
  VkCommandPool pool = VK_NULL_HANDLE;
  CommandPoolState* state = nullptr;
  uint32_t family = 0;
  uint32_t max_timestamps = 0;
  QueueUsage usage = QueueUsage::Graphics;
};

/// @brief Allocates a Vulkan command pool for `queue`.
/// @param queue Queue the buffers will be submitted to
/// @param desc Pool parameters
/// @return The pool, or a recoverable `Error`
/// @warning Asserts in next cases:
/// - If `queue` is null
/// - If `queue` has no device
/// - If command state is not initialized
[[nodiscard]] APERTURE_API auto CreateCommandPool(
    Queue* queue, const aperture::CommandPoolDesc& desc) noexcept
    -> Result<CommandPool*>;

/// @brief Recycles every buffer in `pool`. Does not wait.
/// @param pool Pool to reset. Null is a no-op
/// @warning Asserts in next cases:
/// - If `pool` has no state
/// - If `pool` has no device
/// - If a buffer from `pool` is still recording
APERTURE_API void Reset(CommandPool* pool) noexcept;

/// @brief Destroys `pool`. Does not wait.
/// @param pool Pool to destroy. Null is a no-op
/// @warning Asserts in next cases:
/// - If `pool` has no state
/// - If a buffer from `pool` is still recording
APERTURE_API void Destroy(CommandPool* pool) noexcept;

/// @brief Live command pool for `pool`.
/// @param pool Portable pool handle
/// @return Reference valid until `Destroy(pool)`
/// @warning Asserts in next cases:
/// - If `pool` is null
/// - If `pool` is not Vulkan
[[nodiscard]] APERTURE_API CommandPool& GetNative(
    aperture::CommandPool pool) noexcept;

}  // namespace aperture::vk
