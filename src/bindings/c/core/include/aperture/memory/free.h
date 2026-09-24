#ifndef APERTURE_MEMORY_FREE_H
#define APERTURE_MEMORY_FREE_H

#include <aperture/device.h>
#include <aperture/platform.h>
#include <aperture/types.h>

APERTURE_C_BEGIN

/// @brief Frees a host-mapped allocation. Immediate; does not wait on the GPU.
/// @param device Device that owns the allocation
/// @param ptr Host-mapped allocation to free
/// @warning Asserts if `device` is null when `ptr` is non-null.
APERTURE_C_API void aperture_free(ApertureDevice device,
                                  ApertureDualPtr ptr) APERTURE_C_NOEXCEPT;

/// @brief Frees a device-only allocation. Immediate; does not wait on the GPU.
/// @param device Device that owns the allocation
/// @param ptr Device address to free
/// @warning Asserts if `device` is null when `ptr` is non-null.
APERTURE_C_API void aperture_free_gpu(ApertureDevice device,
                                      ApertureGpuPtr ptr) APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
