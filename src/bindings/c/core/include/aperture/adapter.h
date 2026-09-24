#ifndef APERTURE_ADAPTER_H
#define APERTURE_ADAPTER_H

#include <aperture/capability.h>
#include <aperture/format.h>
#include <aperture/platform.h>
#include <aperture/result.h>
#include <aperture/surface.h>
#include <aperture/swapchain.h>
#include <aperture/types.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

APERTURE_C_BEGIN

/// @brief Frozen adapter record produced by Filter at instance creation.
/// @details `impl` is valid until destroy of the owning instance. `name` and
/// `conformance_reason` point at instance-owned bytes.
typedef struct ApertureAdapter {
  void* impl;
  uint64_t local_memory_bytes;
  uint64_t shared_memory_bytes;
  uint64_t mapped_default_capacity;
  uint64_t texture_heap_alignment;
  ApertureCapability capabilities;
  const char* name;
  const char* conformance_reason;
  size_t name_size;
  size_t conformance_reason_size;
  uint32_t index;
  uint32_t vendor_id;
  uint32_t device_id;
  uint32_t driver_version;
  uint32_t graphics_queue_count;
  uint32_t compute_queue_count;
  uint32_t copy_queue_count;
  uint32_t texture_descriptor_stride;
  uint32_t sampler_descriptor_stride;
  uint32_t max_texture_heap_slots;
  uint32_t max_sampler_heap_slots;
  uint32_t max_texture_dimension_2d;
  uint32_t subgroup_size;
  ApertureTimestampSupport timestamps;
  ApertureCopyGranularity copy_texture_granularity;
  uint8_t uuid[16];
  uint8_t luid[8];
  AperturePresentMode present_modes;
  ApertureError conformance;
  bool discrete;
  bool luid_valid;
} ApertureAdapter;

/// @brief True when the adapter meets the backend floor.
/// @param adapter Adapter to query
/// @return `true` if `adapter` is non-null and `conformance` is `OK`
static inline bool aperture_adapter_conformant(const ApertureAdapter* adapter) {
  return adapter != NULL && adapter->conformance == APERTURE_ERROR_OK;
}

/// @brief True if the adapter reported every bit in `caps`.
/// @param adapter Adapter to query
/// @param caps Required capability bits
/// @return `true` if `adapter` is non-null and has every bit in `caps`
static inline bool aperture_adapter_supports(const ApertureAdapter* adapter,
                                             ApertureCapability caps) {
  return adapter != NULL &&
         aperture_capability_has_all(adapter->capabilities, caps);
}

/// @brief True if `mode` is in `adapter->present_modes`.
/// @param adapter Adapter to query
/// @param mode Present mode to test
/// @return `true` if `adapter` is non-null and reports `mode`
static inline bool aperture_adapter_supports_present(
    const ApertureAdapter* adapter, AperturePresentMode mode) {
  return adapter != NULL &&
         aperture_present_mode_has_all(adapter->present_modes, mode);
}

/// @brief Filter-time format usage query.
/// @param adapter Adapter to query
/// @param format Format to test
/// @param usage Required usage bits
/// @return `true` if `adapter` supports every bit in `usage` for `format`
/// @warning Asserts if `adapter` is null.
APERTURE_C_API bool aperture_supports_format(
    const ApertureAdapter* adapter, ApertureTextureFormat format,
    ApertureFormatUsage usage) APERTURE_C_NOEXCEPT;

/// @brief Presentable color formats for this surface on the adapter.
/// @param adapter Adapter to query
/// @param surface Presentation surface
/// @param data Receives pointer to the format array
/// @param size Receives element count
/// @warning Asserts if `adapter`, `surface`, `data`, or `size` is null.
APERTURE_C_API void aperture_presentable_formats(
    const ApertureAdapter* adapter, const ApertureSurface* surface,
    const ApertureTextureFormat** data, size_t* size) APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
