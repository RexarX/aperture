#pragma once

#include <aperture/command/buffer.hpp>
#include <aperture/platform.hpp>
#include <aperture/result.hpp>
#include <aperture/sync.hpp>
#include <aperture/vulkan/command/pool.hpp>

#include <aperture/vulkan/header.hpp>

namespace aperture::vk {

/// @brief One recording. `Backend` is first. The pool owns the
/// `VkCommandBuffer`.
struct CommandBuffer {
  Backend backend = Backend::Vulkan;
  CommandPool* pool = nullptr;
  VkCommandBuffer buffer = VK_NULL_HANDLE;
  bool recording = false;
};

/// @brief Begins one one-shot command buffer.
/// @param pool Pool that allocates the buffer
/// @return The recording, or `Error::OutOfMemory`
/// @warning Asserts in next cases:
/// - If `pool` is null
/// - If `pool` has no state
/// - If `pool` has no device
[[nodiscard]] APERTURE_API auto Begin(CommandPool* pool) noexcept
    -> Result<CommandBuffer*>;

/// @brief Records one `vkCmdPipelineBarrier2`.
/// @param buffer Recording
/// @param src Source stages
/// @param dst Destination stages
/// @param hazards Extra hazard bits
/// @warning Asserts in next cases:
/// - If `buffer` is null
/// - If `buffer` is not recording
/// - If `buffer` has no pool or device
/// - If `src` is `None`
/// - If `dst` is `None`
/// - If an expanded stage mask is empty
APERTURE_API void Barrier(CommandBuffer* buffer, Stage src, Stage dst,
                          Hazard hazards) noexcept;

/// @brief Records `vkCmdCopyBuffer`.
/// @param buffer Recording
/// @param dst Destination range
/// @param src Source range
/// @warning Asserts in next cases:
/// - If `buffer` is null
/// - If `buffer` is not recording
/// - If `buffer` has no pool or device
/// - If either range is empty
/// - If either address is unknown
/// - If a range extends past its allocation
/// - If the ranges differ in size
/// - If the ranges overlap in one buffer
/// - If the pool queue is missing from an allocation's `QueueUsage`
APERTURE_API void Copy(CommandBuffer* buffer, GpuRange dst,
                       GpuRange src) noexcept;

/// @brief Submits with `vkQueueSubmit2`. Consumes `desc.buffers`.
/// @param queue Queue that runs the buffers
/// @param desc Buffers, waits, and signals
/// @return Nothing, or a recoverable `Error`
/// @warning Asserts in next cases:
/// - If `queue` is null
/// - If `queue` has no device
/// - If command state is not initialized
/// - If a command buffer is null
/// - If a command buffer is not recording
/// - If a command buffer has no pool
/// - If a timeline wait or signal is null
[[nodiscard]] APERTURE_API auto Submit(Queue* queue,
                                       const SubmitDesc& desc) noexcept
    -> Result<void>;

/// @brief Live recording for `buffer`.
/// @param buffer Portable recording token
/// @return Reference valid until `Submit` consumes it
/// @warning Asserts in next cases:
/// - If `buffer` is null
/// - If `buffer` is not Vulkan
[[nodiscard]] APERTURE_API CommandBuffer& GetNative(
    const aperture::CommandBuffer& buffer) noexcept;

}  // namespace aperture::vk
