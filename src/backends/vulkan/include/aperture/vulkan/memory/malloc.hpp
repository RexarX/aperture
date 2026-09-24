#pragma once

#include <aperture/commands.hpp>
#include <aperture/platform.hpp>
#include <aperture/queue.hpp>
#include <aperture/result.hpp>
#include <aperture/types.hpp>
#include <aperture/vulkan/device.hpp>

#include <cstddef>

namespace aperture::vk {

/// @brief Allocates host-mapped device-local memory.
/// @param device Device handle
/// @param bytes Byte count of the allocation
/// @param align Byte alignment of the allocation
/// @param memory Memory type
/// @param usage Queue usage flags
/// @param flags `Dedicated` returns an exclusive block
/// @return Allocated host pointer plus device address, or an error
/// @warning Asserts if `device` is null.
/// @warning Same-`Device` heap ops are externally synchronized.
[[nodiscard]] APERTURE_API auto Malloc(Device* device, size_t bytes,
                                       size_t align, Memory memory,
                                       QueueUsage usage,
                                       MallocFlags flags) noexcept
    -> Result<DualPtr<std::byte>>;

/// @brief Allocates device-only memory.
/// @param device Device handle
/// @param bytes Byte count of the allocation
/// @param align Byte alignment of the allocation
/// @param usage Queue usage flags
/// @param flags `Dedicated` returns an exclusive block
/// @return Allocated device address, or an error
/// @warning Asserts if `device` is null.
/// @warning Same-`Device` heap ops are externally synchronized.
[[nodiscard]] APERTURE_API auto MallocGpu(Device* device, size_t bytes,
                                          size_t align, QueueUsage usage,
                                          MallocFlags flags) noexcept
    -> Result<GpuPtr<std::byte>>;

}  // namespace aperture::vk
