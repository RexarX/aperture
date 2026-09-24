#pragma once

#include <aperture/device.hpp>
#include <aperture/platform.hpp>
#include <aperture/types.hpp>

#include <cstddef>

namespace aperture {

/// @brief Cached device address of a host pointer from `Malloc`.
/// @param device Device that owns the allocation
/// @param host Host pointer returned by `Malloc`
/// @return Device address stored when the allocation was created
/// @warning Asserts if `device` or `host` is null, or if `host` is unknown.
/// @warning Same-`Device` heap ops are externally synchronized.
[[nodiscard]] APERTURE_API auto DeviceAddressOf(Device device,
                                                void* host) noexcept
    -> GpuPtr<std::byte>;

}  // namespace aperture
