#ifndef APERTURE_MEMORY_MALLOC_H
#define APERTURE_MEMORY_MALLOC_H

#include <aperture/commands.h>
#include <aperture/device.h>
#include <aperture/platform.h>
#include <aperture/queue.h>
#include <aperture/result.h>
#include <aperture/types.h>

#include <stddef.h>

APERTURE_C_BEGIN

/// @brief Allocates host-mapped device-local memory.
/// @param device Device that owns the allocation
/// @param bytes Size in bytes
/// @param align Alignment in bytes. Must be a non-zero power of two
/// @param memory `DEFAULT` or `READBACK`
/// @param usage Queues that may touch this allocation
/// @param out Receives host pointer plus device address on success
/// @return The allocation error, or `APERTURE_ERROR_OK`
/// @warning Asserts if `device` or `out` is null.
APERTURE_C_API ApertureError aperture_malloc(
    ApertureDevice device, size_t bytes, size_t align, ApertureMemory memory,
    ApertureQueueUsage usage, ApertureDualPtr* out) APERTURE_C_NOEXCEPT;

/// @brief Allocates an exclusive host-mapped block that other malloc calls
/// will not suballocate from.
/// @param device Device that owns the allocation
/// @param bytes Size in bytes
/// @param align Alignment in bytes. Must be a non-zero power of two
/// @param memory `DEFAULT` or `READBACK`
/// @param usage Queues that may touch this allocation
/// @param out Receives host pointer plus device address on success
/// @return The allocation error, or `APERTURE_ERROR_OK`
/// @warning Asserts if `device` or `out` is null.
APERTURE_C_API ApertureError aperture_malloc_dedicated(
    ApertureDevice device, size_t bytes, size_t align, ApertureMemory memory,
    ApertureQueueUsage usage, ApertureDualPtr* out) APERTURE_C_NOEXCEPT;

/// @brief Allocates device-only memory. Not CPU-writable.
/// @param device Device that owns the allocation
/// @param bytes Size in bytes
/// @param align Alignment in bytes. Must be a non-zero power of two
/// @param usage Queues that may touch this allocation
/// @param out Receives the device address on success
/// @return The allocation error, or `APERTURE_ERROR_OK`
/// @warning Asserts if `device` or `out` is null.
APERTURE_C_API ApertureError aperture_malloc_gpu(
    ApertureDevice device, size_t bytes, size_t align, ApertureQueueUsage usage,
    ApertureGpuPtr* out) APERTURE_C_NOEXCEPT;

/// @brief Allocates an exclusive device-only block.
/// @param device Device that owns the allocation
/// @param bytes Size in bytes
/// @param align Alignment in bytes. Must be a non-zero power of two
/// @param usage Queues that may touch this allocation
/// @param out Receives the device address on success
/// @return The allocation error, or `APERTURE_ERROR_OK`
/// @warning Asserts if `device` or `out` is null.
APERTURE_C_API ApertureError aperture_malloc_gpu_dedicated(
    ApertureDevice device, size_t bytes, size_t align, ApertureQueueUsage usage,
    ApertureGpuPtr* out) APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
