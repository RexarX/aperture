#pragma once

#include <aperture/device.hpp>
#include <aperture/platform.hpp>
#include <aperture/types.hpp>

#include <cstddef>

namespace aperture {

/// @brief Frees a host-mapped allocation. Immediate; does not wait on the GPU.
/// @param device Device that owns the allocation
/// @param ptr Allocation returned by `Malloc`
/// @warning Asserts if `device` is null when `ptr` is non-null.
/// @warning Asserts if `ptr` is not a live `Malloc` result.
/// @warning Same-`Device` `Malloc` / `Free` / `DeviceAddressOf` are externally
/// synchronized. Concurrent calls are a data race; builds with
/// `APERTURE_ENABLE_VALIDATION_SUPPORT` assert.
APERTURE_API void Free(Device device, DualPtr<std::byte> ptr) noexcept;

/// @brief Frees a device-only allocation. Immediate; does not wait on the GPU.
/// @param device Device that owns the allocation
/// @param ptr Allocation returned by `MallocGpu`
/// @warning Asserts if `device` is null when `ptr` is non-null.
/// @warning Asserts if `ptr` is not a live `MallocGpu` result.
/// @warning Same-`Device` heap ops are externally synchronized.
APERTURE_API void Free(Device device, GpuPtr<std::byte> ptr) noexcept;

/// @brief Typed `Free` for a `Malloc` result.
/// @tparam T Element type
/// @param device Device that owns the allocation
/// @param ptr Allocation returned by `Malloc<T>`
template <typename T>
inline void Free(Device device, DualPtr<T> ptr) noexcept {
  Free(device, ptr.template As<std::byte>());
}

/// @brief Typed `Free` for a `MallocGpu` result.
/// @tparam T Element type
/// @param device Device that owns the allocation
/// @param ptr Allocation returned by `MallocGpu<T>`
template <typename T>
inline void Free(Device device, GpuPtr<T> ptr) noexcept {
  Free(device, ptr.template As<std::byte>());
}

}  // namespace aperture
