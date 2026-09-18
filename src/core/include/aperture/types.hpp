#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace aperture {

/// @brief Native graphics API selected at instance creation.
enum class Backend : uint8_t { Vulkan, D3D12, Metal };

/// @brief Name of a `Backend` enumerator.
/// @param backend Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(Backend backend) noexcept {
  switch (backend) {
    using enum Backend;
    case Vulkan:
      return "Vulkan";
    case D3D12:
      return "D3D12";
    case Metal:
      return "Metal";
  }
  return "Unknown";
}

/// @brief 64-bit device address. Matches Slang `GpuPtr<T>`.
/// @tparam T Pointer type on the device
template <typename T>
struct GpuPtr {
  uint64_t addr = 0;

  [[nodiscard]] constexpr bool operator==(const GpuPtr&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const GpuPtr&) const noexcept =
      default;
};

/// @brief Host-mapped allocation plus its device address.
/// @tparam T Element type of the mapped region
template <typename T>
struct DualPtr {
  T* host = nullptr;
  GpuPtr<T> device;

  [[nodiscard]] constexpr bool operator==(const DualPtr&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const DualPtr&) const noexcept =
      default;
};

/// @brief Device address plus byte count. Pass by value.
/// @details `size` is always bytes, independent of element type. Command
/// recording consumes this directly — copies, index binds, and indirect
/// arguments do not look up a buffer object.
struct GpuRange {
  GpuPtr<std::byte> gpu;
  uint64_t size = 0;

  [[nodiscard]] constexpr bool operator==(const GpuRange&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const GpuRange&) const noexcept =
      default;
};

/// @brief `count * sizeof(T)` bytes starting at `ptr`.
/// @tparam T Element type
/// @param ptr Device pointer
/// @param count Element count (default: 1)
/// @return Range covering those elements
template <typename T>
[[nodiscard]] constexpr GpuRange GpuRangeFrom(GpuPtr<T> ptr,
                                              uint64_t count = 1) noexcept {
  return {.gpu = {.addr = ptr.addr}, .size = count * sizeof(T)};
}

/// @brief `count * sizeof(T)` bytes starting at `ptr.device`.
/// @tparam T Element type
/// @param ptr Mapped allocation
/// @param count Element count (default: 1)
/// @return Range covering those elements
template <typename T>
[[nodiscard]] constexpr GpuRange GpuRangeFrom(DualPtr<T> ptr,
                                              uint64_t count = 1) noexcept {
  return GpuRangeFrom(ptr.device, count);
}

}  // namespace aperture
