#pragma once

#include <aperture/device.hpp>
#include <aperture/platform.hpp>
#include <aperture/types.hpp>

#include <aperture/vulkan/header.hpp>

#include <cstddef>
#include <cstdint>

namespace aperture::vk {

/// @brief Spanning `VkBuffer` plus a byte range.
struct Buffer {
  VkBuffer buffer = VK_NULL_HANDLE;
  uint64_t address = 0;
  uint64_t offset = 0;
  uint64_t size = 0;
};

/// @brief Spanning buffer that contains `ptr`.
/// @param device Portable device handle
/// @param ptr Device address from `Malloc`, `MallocGpu`, or an offset into
/// either
/// @return Buffer handle plus offset/size of the allocation
/// @warning Asserts if `device` is null or `ptr` is unknown.
/// @warning Same-`Device` heap ops are externally synchronized.
[[nodiscard]] APERTURE_API Buffer GetVkBuffer(aperture::Device device,
                                              GpuPtr<std::byte> ptr) noexcept;

/// @brief Cached device address of a `Malloc` host pointer.
/// @param device Portable device handle
/// @param host Host pointer
/// @warning Asserts if `device` or `host` is null, or if `host` is unknown.
/// @warning Same-`Device` heap ops are externally synchronized.
[[nodiscard]] APERTURE_API auto DeviceAddressOf(Device* device,
                                                void* host) noexcept
    -> GpuPtr<std::byte>;

}  // namespace aperture::vk
