#ifndef APERTURE_VULKAN_MEMORY_MALLOC_H
#define APERTURE_VULKAN_MEMORY_MALLOC_H

#include <aperture/device.h>
#include <aperture/memory/malloc.h>
#include <aperture/platform.h>
#include <aperture/queue.h>
#include <aperture/result.h>
#include <aperture/types.h>

#include <stddef.h>

APERTURE_C_BEGIN

/// @brief Vulkan-backend host-mapped malloc. Same contract as
/// `aperture_malloc`.
/// @param device Device that owns the allocation
/// @param bytes Size in bytes
/// @param align Alignment in bytes. Must be a non-zero power of two
/// @param memory `DEFAULT` or `READBACK`
/// @param usage Queues that may touch this allocation
/// @param flags `DEDICATED` returns an exclusive block
/// @param out Receives host pointer plus device address on success
/// @return The allocation error, or `APERTURE_ERROR_OK`
/// @warning Asserts if `device` or `out` is null.
APERTURE_C_API ApertureError aperture_vk_malloc(
    ApertureDevice device, size_t bytes, size_t align, ApertureMemory memory,
    ApertureQueueUsage usage, ApertureMallocFlags flags,
    ApertureDualPtr* out) APERTURE_C_NOEXCEPT;

/// @brief Vulkan-backend device-only malloc.
/// @param device Device that owns the allocation
/// @param bytes Size in bytes
/// @param align Alignment in bytes. Must be a non-zero power of two
/// @param usage Queues that may touch this allocation
/// @param flags `DEDICATED` returns an exclusive block
/// @param out Receives the device address on success
/// @return The allocation error, or `APERTURE_ERROR_OK`
/// @warning Asserts if `device` or `out` is null.
APERTURE_C_API ApertureError aperture_vk_malloc_gpu(
    ApertureDevice device, size_t bytes, size_t align, ApertureQueueUsage usage,
    ApertureMallocFlags flags, ApertureGpuPtr* out) APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
