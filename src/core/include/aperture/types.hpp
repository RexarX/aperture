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

/// @brief Native compiled-shader encoding of a `ShaderBlob`.
/// @details Must match `NativeShaderFormat(BackendOf(device))`. A mismatch,
/// `Invalid`, or a blob whose magic does not match the declared format is
/// `Error::Invalid` at `CreateProgram`. Default is `Invalid` so omitting
/// `format` cannot silently select the wrong backend encoding.
enum class ShaderFormat : uint8_t {
  Invalid = 0,
  Spirv,
  Dxil,
  MetalLib,
};

/// @brief SPIR-V magic (little-endian). A `Spirv` blob's first word must be
/// this or `SPIRV_MAGIC_SWAPPED`.
inline constexpr uint32_t SPIRV_MAGIC = 0x07230203;

/// @brief SPIR-V magic with byte-swapped endianness.
inline constexpr uint32_t SPIRV_MAGIC_SWAPPED = 0x03022307;

/// @brief DXBC container magic (`'DXBC'`). A `Dxil` blob starts with this.
inline constexpr uint32_t DXBC_MAGIC = 0x43425844;

/// @brief Metal library magic (`'MTLB'`). A `MetalLib` blob starts with this.
inline constexpr uint32_t MTLB_MAGIC = 0x424C544D;

/// @brief Name of a `ShaderFormat` enumerator.
/// @param format Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(
    ShaderFormat format) noexcept {
  switch (format) {
    using enum ShaderFormat;
    case Invalid:
      return "Invalid";
    case Spirv:
      return "Spirv";
    case Dxil:
      return "Dxil";
    case MetalLib:
      return "MetalLib";
  }
  return "Unknown";
}

/// @brief Native blob encoding consumed by `backend`.
/// @param backend Instance / device backend
/// @return Format that `CreateProgram` accepts on that backend
[[nodiscard]] constexpr ShaderFormat NativeShaderFormat(
    Backend backend) noexcept {
  switch (backend) {
    using enum Backend;
    case Vulkan:
      return ShaderFormat::Spirv;
    case D3D12:
      return ShaderFormat::Dxil;
    case Metal:
      return ShaderFormat::MetalLib;
  }
  return ShaderFormat::Invalid;
}

/// @brief True when `format` is the encoding `backend` consumes.
/// @param format Declared blob format
/// @param backend Instance / device backend
/// @return `false` for `Invalid` or a cross-backend encoding
[[nodiscard]] constexpr bool Compatible(ShaderFormat format,
                                        Backend backend) noexcept {
  return format != ShaderFormat::Invalid &&
         format == NativeShaderFormat(backend);
}

/// @brief Texel-block counts for copy-queue texture copies.
/// @details A zero component means that axis must cover the whole mip.
struct CopyGranularity {
  uint32_t x = 1;
  uint32_t y = 1;
  uint32_t z = 1;
};

/// @brief 64-bit device address. Matches the shader-visible `GpuPtr<T>` on
/// every addressing profile.
/// @tparam T Pointer type on the device
template <typename T>
struct GpuPtr {
  uint64_t addr = 0;

  /// @brief Reinterprets this address as `U`.
  /// @tparam U New pointee type
  /// @return The same address
  template <typename U>
  [[nodiscard]] constexpr auto As() const noexcept -> GpuPtr<U> {
    return {.addr = addr};
  }

  [[nodiscard]] constexpr bool operator==(const GpuPtr&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const GpuPtr&) const noexcept =
      default;
};

/// @brief Advances `ptr` by `count` elements.
/// @tparam T Pointee type
/// @param ptr Device address
/// @param count Element count
/// @return Address `count * sizeof(T)` bytes past `ptr`
/// @warning `count * sizeof(T)` must not overflow.
template <typename T>
[[nodiscard]] constexpr auto operator+(GpuPtr<T> ptr, uint64_t count) noexcept
    -> GpuPtr<T> {
  return {.addr = ptr.addr + count * sizeof(T)};
}

/// @brief Host-mapped allocation plus its device address.
/// @tparam T Element type of the mapped region
template <typename T>
struct DualPtr {
  T* host = nullptr;
  GpuPtr<T> device;

  /// @brief Reinterprets this allocation as `U`.
  /// @tparam U New pointee type
  /// @return Host and device addresses with the new type
  template <typename U>
  [[nodiscard]] auto As() const noexcept -> DualPtr<U> {
    return {
        .host = reinterpret_cast<U*>(host),
        .device = device.template As<U>(),
    };
  }

  [[nodiscard]] constexpr bool operator==(const DualPtr&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const DualPtr&) const noexcept =
      default;
};

/// @brief Advances both addresses by `count` elements.
/// @tparam T Pointee type
/// @param ptr Mapped allocation
/// @param count Element count
/// @return Host and device addresses `count * sizeof(T)` bytes past `ptr`
/// @warning `count * sizeof(T)` must not overflow.
template <typename T>
[[nodiscard]] constexpr auto operator+(DualPtr<T> ptr, uint64_t count) noexcept
    -> DualPtr<T> {
  return {.host = ptr.host + count, .device = ptr.device + count};
}

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

/// @brief Host-mapped pointer, device address, and byte count. Pass by value.
/// @details `size` is always bytes, independent of element type. Same shape as
/// `GpuRange`, with a host view for CPU writes / reads.
struct DualRange {
  DualPtr<std::byte> ptr;
  uint64_t size = 0;

  [[nodiscard]] constexpr bool operator==(const DualRange&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const DualRange&) const noexcept =
      default;

  /// @brief Device-only view of this range.
  [[nodiscard]] constexpr GpuRange Device() const noexcept {
    return {.gpu = ptr.device, .size = size};
  }
};

/// @brief `count * sizeof(T)` bytes starting at `ptr`.
/// @tparam T Element type
/// @param ptr Mapped allocation
/// @param count Element count (default: 1)
/// @return Range covering those elements
template <typename T>
[[nodiscard]] inline DualRange DualRangeFrom(DualPtr<T> ptr,
                                             uint64_t count = 1) noexcept {
  return {.ptr = ptr.template As<std::byte>(), .size = count * sizeof(T)};
}

/// @brief Subrange of `range` starting `offset` bytes in.
/// @param range Host-mapped range
/// @param offset Byte offset from the start of `range`
/// @param bytes Byte count of the subrange
/// @return Host and device addresses advanced by `offset`
[[nodiscard]] inline DualRange Slice(DualRange range, uint64_t offset,
                                     uint64_t bytes) noexcept {
  return {.ptr = range.ptr + offset, .size = bytes};
}

}  // namespace aperture
