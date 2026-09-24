#pragma once

#include <aperture/capability.hpp>
#include <aperture/instance.hpp>
#include <aperture/pipeline.hpp>
#include <aperture/platform.hpp>
#include <aperture/result.hpp>
#include <aperture/types.hpp>

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace aperture {

/// @brief Pointer-sized CPU handle for a logical device.
struct Device {
  void* ptr = nullptr;

  /// @brief True if this handle is non-null.
  /// @return `true` when `ptr != nullptr`
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return ptr != nullptr;
  }

  [[nodiscard]] constexpr bool operator==(const Device&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const Device&) const noexcept =
      default;
};

/// @brief How shaders address resources on this device.
enum class AddressingProfile : uint8_t { Pointer, Handle };

/// @brief Name of an `AddressingProfile` enumerator.
/// @param profile Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(
    AddressingProfile profile) noexcept {
  switch (profile) {
    using enum AddressingProfile;
    case Pointer:
      return "Pointer";
    case Handle:
      return "Handle";
  }
  return "Unknown";
}

/// @brief Parameters for `CreateDevice`. Copied; the call does not retain
/// references.
struct DeviceDesc {
  Capability capabilities = Capability::None;
  uint32_t adapter = 0;
  uint32_t texture_heap_slots = 0;
  uint32_t sampler_heap_slots = 0;
  PipelinePolicy pipeline_policy = PipelinePolicy::FailOnMiss;
};

/// @brief Device constants frozen at create time.
/// @details The reference returned by `Info` is valid until `Destroy(device)`.
struct DeviceInfo {
  GpuPtr<std::byte> texture_heap_device;
  GpuPtr<std::byte> sampler_heap_device;
  /// Power-of-two quantum. Every texture `SizeAlign::align` divides it.
  uint64_t texture_heap_alignment = 0;
  /// Nanoseconds per timestamp tick. Zero when timestamps are unsupported.
  float timestamp_period_ns = 0.0F;
  uint32_t texture_descriptor_stride = 0;
  uint32_t sampler_descriptor_stride = 0;
  uint32_t texture_heap_slots = 0;
  uint32_t sampler_heap_slots = 0;
  uint32_t null_texture_slot = 0;
  uint32_t null_sampler_slot = 0;
  /// Push-data / CPU-root byte limit (`min(maxPushDataSize, 256)` on Vulkan).
  uint32_t max_cpu_root_bytes = 0;
  CopyGranularity copy_texture_granularity;
  AddressingProfile profile = AddressingProfile::Pointer;
  bool graphics_timestamps = false;
  bool compute_timestamps = false;
  bool copy_timestamps = false;
};

/// @brief Native API of this device.
/// @param device Device to query
/// @return Backend stored in the device
/// @warning Asserts if `device` is null.
[[nodiscard]] APERTURE_API Backend BackendOf(Device device) noexcept;

/// @brief Creates a logical device on `desc.adapter`.
/// @param instance Instance that owns `desc.adapter`
/// @param desc Device creation parameters
/// @return The device, or a recoverable `Error`
/// @warning Heap slot counts must be >= 1. Non-conformant adapters fail.
/// @warning Asserts if `instance` is null.
[[nodiscard]] APERTURE_API auto CreateDevice(Instance instance,
                                             const DeviceDesc& desc) noexcept
    -> Result<Device>;

/// @brief Destroys a device. Immediate; does not wait on the GPU.
/// @param device Device to destroy. Null is a no-op.
APERTURE_API void Destroy(Device device) noexcept;

/// @brief Capabilities enabled at `CreateDevice`, not merely reported.
/// @param device Device to query
/// @param caps Required capability bits
/// @return `true` if every bit in `caps` is enabled on `device`
/// @warning Asserts if `device` is null.
[[nodiscard]] APERTURE_API bool Has(Device device, Capability caps) noexcept;

/// @brief Device constants frozen at create time.
/// @param device Device to query
/// @return Reference valid until `Destroy(device)`
/// @warning Asserts if `device` is null.
[[nodiscard]] APERTURE_API const DeviceInfo& Info(Device device) noexcept;

}  // namespace aperture
