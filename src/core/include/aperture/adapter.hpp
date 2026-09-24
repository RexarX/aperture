#pragma once

#include <aperture/capability.hpp>
#include <aperture/format.hpp>
#include <aperture/platform.hpp>
#include <aperture/result.hpp>
#include <aperture/surface.hpp>
#include <aperture/swapchain.hpp>
#include <aperture/types.hpp>

#include <cstdint>
#include <span>
#include <string>

namespace aperture {

/// @brief Frozen adapter record produced by Filter at instance creation.
/// @details `impl` is valid until `Destroy` of the owning instance. `name` and
/// `conformance_reason` are owned copies and remain valid for the lifetime of
/// this record. Strings come first, then pointers and 8-byte scalars, then
/// 32-bit fields, then bytes.
struct Adapter {
  std::string name;
  std::string conformance_reason;
  /// Opaque backend record. Valid until `Destroy` of the owning instance.
  void* impl = nullptr;
  uint64_t local_memory_bytes = 0;
  uint64_t shared_memory_bytes = 0;
  uint64_t mapped_default_capacity = 0;
  /// Power-of-two quantum. Every texture `SizeAlign::align` divides it.
  uint64_t texture_heap_alignment = 0;
  Capability capabilities = Capability::None;
  uint32_t index = 0;
  uint32_t vendor_id = 0;
  uint32_t device_id = 0;
  uint32_t driver_version = 0;
  /// Hardware capacity. v1 creates one queue per enabled usage.
  uint32_t graphics_queue_count = 0;
  uint32_t compute_queue_count = 0;
  uint32_t copy_queue_count = 0;
  uint32_t texture_descriptor_stride = 0;
  uint32_t sampler_descriptor_stride = 0;
  uint32_t max_texture_heap_slots = 0;
  uint32_t max_sampler_heap_slots = 0;
  uint32_t max_texture_dimension_2d = 0;
  uint32_t subgroup_size = 0;
  TimestampSupport timestamps;
  CopyGranularity copy_texture_granularity;
  uint8_t uuid[16] = {};
  uint8_t luid[8] = {};
  PresentMode present_modes = PresentMode::None;
  Error conformance = Error::Ok;
  bool discrete = false;
  bool luid_valid = false;
};

/// @brief True when the adapter meets the backend floor.
/// @param adapter Adapter to query
/// @return `true` if `conformance == Error::Ok`
[[nodiscard]] constexpr bool Conformant(const Adapter& adapter) noexcept {
  return adapter.conformance == Error::Ok;
}

/// @brief True if the adapter reported every bit in `caps`.
/// @param adapter Adapter to query
/// @param caps Required capability bits
/// @return `true` if `HasAll(adapter.capabilities, caps)`
[[nodiscard]] constexpr bool Supports(const Adapter& adapter,
                                      Capability caps) noexcept {
  return HasAll(adapter.capabilities, caps);
}

/// @brief Filter-time format usage query.
/// @param adapter Adapter to query
/// @param format Format to test
/// @param usage Required usage bits
/// @return `true` if `adapter` supports every bit in `usage` for `format`
[[nodiscard]] APERTURE_API bool SupportsFormat(const Adapter& adapter,
                                               TextureFormat format,
                                               FormatUsage usage) noexcept;

/// @brief True if `mode` is in `Adapter::present_modes`.
/// @param adapter Adapter to query
/// @param mode Present mode to test
/// @return `true` if `HasAll(adapter.present_modes, mode)`
[[nodiscard]] constexpr bool SupportsPresent(const Adapter& adapter,
                                             PresentMode mode) noexcept {
  return HasAll(adapter.present_modes, mode);
}

/// @brief Presentable color formats for this surface on the adapter.
/// @details Span valid until the next `PresentableFormats` call on this
/// adapter or `Destroy` of the owning instance.
/// @param adapter Adapter to query
/// @param surface Presentation surface to present to
/// @return Presentable color formats, or an empty span if none apply
[[nodiscard]] APERTURE_API auto PresentableFormats(
    const Adapter& adapter, const Surface& surface) noexcept
    -> std::span<const TextureFormat>;

}  // namespace aperture
