#pragma once

#include <aperture/device.hpp>
#include <aperture/memory/common.hpp>
#include <aperture/platform.hpp>
#include <aperture/queue.hpp>
#include <aperture/result.hpp>
#include <aperture/types.hpp>
#include <aperture/utils/bit.hpp>
#include <aperture/utils/flags.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string_view>

namespace aperture {

/// @brief Placement flags for `Malloc` and `MallocGpu`.
enum class MallocFlags : uint32_t {
  None = 0,
  Dedicated = 1U << 0U,
};

/// @brief Name of a `MallocFlags` enumerator.
/// @param flags Exact enumerator or `None`. Combined masks return `"Flags"`.
/// @return Enumerator name, `"Flags"`, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(MallocFlags flags) noexcept {
  switch (flags) {
    using enum MallocFlags;
    case None:
      return "None";
    case Dedicated:
      return "Dedicated";
  }
  return "Flags";
}

template <>
inline constexpr bool IS_FLAGS<MallocFlags> = true;

/// @brief Allocates host-mapped device-local memory.
/// @details `Default` is write-combined and host-coherent. `Readback` is
/// host-cached. Translation to a device address is done once and cached.
/// Every result is aligned to at least 16 bytes, even when `align` is
/// smaller.
/// @param device Device that owns the allocation
/// @param bytes Size in bytes
/// @param align Alignment in bytes. Must be a non-zero power of two
/// @param memory `Default` or `Readback`
/// @param usage Queues that may touch this allocation
/// @return Host pointer plus device address, or a recoverable `Error`
/// @warning Asserts if `device` is null.
/// @warning Extra `usage` bits the device did not enable fail `Unsupported`.
/// @param flags `Dedicated` returns an exclusive block other heap calls will
/// not suballocate from
/// @warning Same-`Device` `Malloc` / `MallocGpu` / `Free` / `DeviceAddressOf`
/// are externally synchronized. Concurrent calls are a data race; builds with
/// `APERTURE_ENABLE_VALIDATION_SUPPORT` assert.
[[nodiscard]] APERTURE_API auto Malloc(
    Device device, size_t bytes, size_t align, Memory memory = Memory::Default,
    QueueUsage usage = QueueUsage::Graphics,
    MallocFlags flags = MallocFlags::None) noexcept
    -> Result<DualPtr<std::byte>>;

/// @brief Typed `Malloc`: `count * sizeof(T)` bytes at `alignof(T)`.
/// @tparam T Element type
/// @param device Device that owns the allocation
/// @param count Element count (default: 1)
/// @param memory `Default` or `Readback`
/// @param usage Queues that may touch this allocation
/// @param flags `Dedicated` returns an exclusive block
/// @return Typed host-plus-device pointer, or a recoverable `Error`
/// @warning Asserts if `device` is null.
template <typename T>
[[nodiscard]] inline auto Malloc(Device device, size_t count = 1,
                                 Memory memory = Memory::Default,
                                 QueueUsage usage = QueueUsage::Graphics,
                                 MallocFlags flags = MallocFlags::None) noexcept
    -> Result<DualPtr<T>> {
  if (count != 0 && utils::MulOverflows(sizeof(T), count)) [[unlikely]] {
    return std::unexpected(Error::Invalid);
  }

  auto allocated =
      Malloc(device, count * sizeof(T), alignof(T), memory, usage, flags);
  if (!allocated) [[unlikely]] {
    return std::unexpected(allocated.error());
  }
  return DualPtr<T>{
      .host = reinterpret_cast<T*>(allocated->host),
      .device = {.addr = allocated->device.addr},
  };
}

/// @brief Allocates device-only memory. Not CPU-writable.
/// @param device Device that owns the allocation
/// @param bytes Size in bytes. Must be non-zero
/// @param align Alignment in bytes. Must be a non-zero power of two
/// @param usage Queues that may touch this allocation
/// @param flags `Dedicated` returns an exclusive block other heap calls will
/// not suballocate from
/// @return Device address, or a recoverable `Error`
/// @warning Asserts if `device` is null.
/// @warning Extra `usage` bits the device did not enable fail `Unsupported`.
/// @warning Same-`Device` heap ops are externally synchronized.
[[nodiscard]] APERTURE_API auto MallocGpu(
    Device device, size_t bytes, size_t align,
    QueueUsage usage = QueueUsage::Graphics,
    MallocFlags flags = MallocFlags::None) noexcept
    -> Result<GpuPtr<std::byte>>;

/// @brief Typed `MallocGpu`: `count * sizeof(T)` bytes at `alignof(T)`.
/// @tparam T Element type
/// @param device Device that owns the allocation
/// @param count Element count (default: 1)
/// @param usage Queues that may touch this allocation
/// @param flags `Dedicated` returns an exclusive block
/// @return Typed device pointer, or a recoverable `Error`
/// @warning Asserts if `device` is null.
template <typename T>
[[nodiscard]] inline auto MallocGpu(
    Device device, size_t count = 1, QueueUsage usage = QueueUsage::Graphics,
    MallocFlags flags = MallocFlags::None) noexcept -> Result<GpuPtr<T>> {
  if (count != 0 && utils::MulOverflows(sizeof(T), count)) [[unlikely]] {
    return std::unexpected(Error::Invalid);
  }

  const auto allocated =
      MallocGpu(device, count * sizeof(T), alignof(T), usage, flags);
  if (!allocated) [[unlikely]] {
    return std::unexpected(allocated.error());
  }
  return GpuPtr<T>{.addr = allocated->addr};
}

}  // namespace aperture
