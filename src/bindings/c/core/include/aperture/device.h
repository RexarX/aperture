#ifndef APERTURE_DEVICE_H
#define APERTURE_DEVICE_H

#include <aperture/capability.h>
#include <aperture/instance.h>
#include <aperture/pipeline.h>
#include <aperture/platform.h>
#include <aperture/result.h>
#include <aperture/types.h>

#include <stdbool.h>
#include <stdint.h>

APERTURE_C_BEGIN

/// @brief Pointer-sized CPU handle for a logical device. Invalid is `NULL`.
typedef struct ApertureDeviceImpl* ApertureDevice;

/// @brief How shaders address resources on this device.
typedef uint8_t ApertureAddressingProfile;

enum {
  APERTURE_ADDRESSING_PROFILE_POINTER = 0U,
  APERTURE_ADDRESSING_PROFILE_HANDLE,
};

/// @brief Name of an `ApertureAddressingProfile` enumerator.
/// @param profile Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
static inline const char* aperture_addressing_profile_to_string(
    ApertureAddressingProfile profile) {
  switch (profile) {
    case APERTURE_ADDRESSING_PROFILE_POINTER:
      return "Pointer";
    case APERTURE_ADDRESSING_PROFILE_HANDLE:
      return "Handle";
    default:
      return "Unknown";
  }
}

/// @brief Parameters for `aperture_create_device`.
typedef struct ApertureDeviceDesc {
  ApertureCapability capabilities;
  uint32_t adapter;
  uint32_t texture_heap_slots;
  uint32_t sampler_heap_slots;
  AperturePipelinePolicy pipeline_policy;
} ApertureDeviceDesc;

/// @brief Device constants frozen at create time.
typedef struct ApertureDeviceInfo {
  ApertureGpuPtr texture_heap_device;
  ApertureGpuPtr sampler_heap_device;
  uint64_t texture_heap_alignment;
  float timestamp_period_ns;
  uint32_t texture_descriptor_stride;
  uint32_t sampler_descriptor_stride;
  uint32_t texture_heap_slots;
  uint32_t sampler_heap_slots;
  uint32_t null_texture_slot;
  uint32_t null_sampler_slot;
  uint32_t max_cpu_root_bytes;
  ApertureCopyGranularity copy_texture_granularity;
  ApertureAddressingProfile profile;
  bool graphics_timestamps;
  bool compute_timestamps;
  bool copy_timestamps;
} ApertureDeviceInfo;

/// @brief Native API of this device.
/// @param device Device to query
/// @return Backend stored in the device
/// @warning Asserts if `device` is null.
APERTURE_C_API ApertureBackend aperture_backend_of_device(ApertureDevice device)
    APERTURE_C_NOEXCEPT;

/// @brief Creates a logical device on `desc->adapter`.
/// @param instance Instance that owns `desc->adapter`
/// @param desc Device creation parameters
/// @param out Receives the device on success
/// @return The device error, or `APERTURE_ERROR_OK`
/// @warning Heap slot counts must be >= 1. Non-conformant adapters fail.
/// @warning Asserts if `instance`, `desc`, or `out` is null.
APERTURE_C_API ApertureError aperture_create_device(
    ApertureInstance instance, const ApertureDeviceDesc* desc,
    ApertureDevice* out) APERTURE_C_NOEXCEPT;

/// @brief Destroys a device. Immediate; does not wait on the GPU. Null is a
/// no-op.
/// @param device Device to destroy
APERTURE_C_API void aperture_destroy_device(ApertureDevice device)
    APERTURE_C_NOEXCEPT;

/// @brief Capabilities enabled at create, not merely reported.
/// @param device Device to query
/// @param caps Required capability bits
/// @return `true` if every bit in `caps` is enabled
/// @warning Asserts if `device` is null.
APERTURE_C_API bool aperture_device_has(
    ApertureDevice device, ApertureCapability caps) APERTURE_C_NOEXCEPT;

/// @brief Device constants frozen at create time.
/// @param device Device to query
/// @return Copy of the frozen device info
/// @warning Asserts if `device` is null.
APERTURE_C_API ApertureDeviceInfo aperture_device_info(ApertureDevice device)
    APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
