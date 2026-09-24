#pragma once

#include <aperture/commands.hpp>
#include <aperture/device.hpp>
#include <aperture/platform.hpp>
#include <aperture/queue.hpp>
#include <aperture/result.hpp>
#include <aperture/types.hpp>
#include <aperture/utils/bit.hpp>

#include <cstddef>
#include <expected>

namespace aperture {

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
/// @warning Same-`Device` `Malloc` / `MallocGpu` / `MallocDedicated` / `Free` /
/// `DeviceAddressOf` are externally synchronized. Concurrent calls are a data
/// race; builds with `APERTURE_ENABLE_VALIDATION_SUPPORT` assert.
[[nodiscard]] APERTURE_API auto Malloc(
    Device device, size_t bytes, size_t align, Memory memory = Memory::Default,
    QueueUsage usage = QueueUsage::Graphics) noexcept
    -> Result<DualPtr<std::byte>>;

/// @brief Typed `Malloc`: `count * sizeof(T)` bytes at `alignof(T)`.
/// @tparam T Element type
/// @param device Device that owns the allocation
/// @param count Element count (default: 1)
/// @param memory `Default` or `Readback`
/// @param usage Queues that may touch this allocation
/// @return Typed host-plus-device pointer, or a recoverable `Error`
/// @warning Asserts if `device` is null.
template <typename T>
[[nodiscard]] inline auto Malloc(
    Device device, size_t count = 1, Memory memory = Memory::Default,
    QueueUsage usage = QueueUsage::Graphics) noexcept -> Result<DualPtr<T>> {
  if (count != 0 && utils::MulOverflows(sizeof(T), count)) [[unlikely]] {
    return std::unexpected(Error::Invalid);
  }

  auto allocated = Malloc(device, count * sizeof(T), alignof(T), memory, usage);
  if (!allocated) [[unlikely]] {
    return std::unexpected(allocated.error());
  }
  return DualPtr<T>{
      .host = reinterpret_cast<T*>(allocated->host),
      .device = {.addr = allocated->device.addr},
  };
}

/// @brief Allocates an exclusive host-mapped block that other `Malloc` calls
/// will not suballocate from.
/// @details Block size is the request (rounded for alignment), not the shared
/// 64 MiB pool quantum. Intended for parallel loaders: obtain a dedicated
/// range on one thread, then suballocate with a user-owned `BumpAllocator` or
/// `OffsetAllocator`.
/// @param device Device that owns the allocation
/// @param bytes Size in bytes
/// @param align Alignment in bytes. Must be a non-zero power of two
/// @param memory `Default` or `Readback`
/// @param usage Queues that may touch this allocation
/// @return Host pointer plus device address, or a recoverable `Error`
/// @warning Asserts if `device` is null.
/// @warning Same-`Device` heap ops are externally synchronized.
[[nodiscard]] APERTURE_API auto MallocDedicated(
    Device device, size_t bytes, size_t align, Memory memory = Memory::Default,
    QueueUsage usage = QueueUsage::Graphics) noexcept
    -> Result<DualPtr<std::byte>>;

/// @brief Typed `MallocDedicated`: `count * sizeof(T)` bytes at `alignof(T)`.
/// @tparam T Element type
/// @param device Device that owns the allocation
/// @param count Element count (default: 1)
/// @param memory `Default` or `Readback`
/// @param usage Queues that may touch this allocation
/// @return Typed host-plus-device pointer, or a recoverable `Error`
/// @warning Asserts if `device` is null.
template <typename T>
[[nodiscard]] inline auto MallocDedicated(
    Device device, size_t count = 1, Memory memory = Memory::Default,
    QueueUsage usage = QueueUsage::Graphics) noexcept -> Result<DualPtr<T>> {
  if (count != 0 && utils::MulOverflows(sizeof(T), count)) [[unlikely]] {
    return std::unexpected(Error::Invalid);
  }

  auto allocated =
      MallocDedicated(device, count * sizeof(T), alignof(T), memory, usage);
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
/// @return Device address, or a recoverable `Error`
/// @warning Asserts if `device` is null.
/// @warning Extra `usage` bits the device did not enable fail `Unsupported`.
/// @warning Same-`Device` heap ops are externally synchronized.
[[nodiscard]] APERTURE_API auto MallocGpu(
    Device device, size_t bytes, size_t align,
    QueueUsage usage = QueueUsage::Graphics) noexcept
    -> Result<GpuPtr<std::byte>>;

/// @brief Typed `MallocGpu`: `count * sizeof(T)` bytes at `alignof(T)`.
/// @tparam T Element type
/// @param device Device that owns the allocation
/// @param count Element count (default: 1)
/// @param usage Queues that may touch this allocation
/// @return Typed device pointer, or a recoverable `Error`
/// @warning Asserts if `device` is null.
template <typename T>
[[nodiscard]] inline auto MallocGpu(
    Device device, size_t count = 1,
    QueueUsage usage = QueueUsage::Graphics) noexcept -> Result<GpuPtr<T>> {
  if (count != 0 && utils::MulOverflows(sizeof(T), count)) [[unlikely]] {
    return std::unexpected(Error::Invalid);
  }

  const auto allocated =
      MallocGpu(device, count * sizeof(T), alignof(T), usage);
  if (!allocated) [[unlikely]] {
    return std::unexpected(allocated.error());
  }
  return GpuPtr<T>{.addr = allocated->addr};
}

/// @brief Allocates an exclusive device-only block that other `MallocGpu`
/// calls will not suballocate from.
/// @param device Device that owns the allocation
/// @param bytes Size in bytes. Must be non-zero
/// @param align Alignment in bytes. Must be a non-zero power of two
/// @param usage Queues that may touch this allocation
/// @return Device address, or a recoverable `Error`
/// @warning Asserts if `device` is null.
/// @warning Same-`Device` heap ops are externally synchronized.
[[nodiscard]] APERTURE_API auto MallocGpuDedicated(
    Device device, size_t bytes, size_t align,
    QueueUsage usage = QueueUsage::Graphics) noexcept
    -> Result<GpuPtr<std::byte>>;

/// @brief Typed `MallocGpuDedicated`: `count * sizeof(T)` at `alignof(T)`.
/// @tparam T Element type
/// @param device Device that owns the allocation
/// @param count Element count (default: 1)
/// @param usage Queues that may touch this allocation
/// @return Typed device pointer, or a recoverable `Error`
/// @warning Asserts if `device` is null.
template <typename T>
[[nodiscard]] inline auto MallocGpuDedicated(
    Device device, size_t count = 1,
    QueueUsage usage = QueueUsage::Graphics) noexcept -> Result<GpuPtr<T>> {
  if (count != 0 && utils::MulOverflows(sizeof(T), count)) [[unlikely]] {
    return std::unexpected(Error::Invalid);
  }

  const auto allocated =
      MallocGpuDedicated(device, count * sizeof(T), alignof(T), usage);
  if (!allocated) [[unlikely]] {
    return std::unexpected(allocated.error());
  }
  return GpuPtr<T>{.addr = allocated->addr};
}

}  // namespace aperture
