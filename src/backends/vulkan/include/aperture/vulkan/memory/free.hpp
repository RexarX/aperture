#pragma once

#include <aperture/platform.hpp>
#include <aperture/types.hpp>
#include <aperture/vulkan/device.hpp>

namespace aperture::vk {

/// @brief Frees a host-mapped allocation. Both-null `ptr` is a no-op.
/// @param device Portable device handle
/// @param ptr Host pointer plus device address from `Malloc`
/// @warning Asserts if `device` is null when `ptr` is non-null.
/// @warning Same-`Device` heap ops are externally synchronized.
APERTURE_API void Free(Device* device, DualPtr<std::byte> ptr) noexcept;

/// @brief Frees a device-only allocation. Zero `ptr` is a no-op.
/// @param device Portable device handle
/// @param ptr Device address from `MallocGpu`
/// @warning Asserts if `device` is null when `ptr` is non-null.
/// @warning Same-`Device` heap ops are externally synchronized.
APERTURE_API void Free(Device* device, GpuPtr<std::byte> ptr) noexcept;

}  // namespace aperture::vk
