#pragma once

#include <cstdint>
#include <string_view>
#include <utility>

namespace aperture {

/// @brief Closed texture format set.
/// @details Values may grow; existing values stay stable across versions.
enum class TextureFormat : uint8_t {
  Undefined = 0,
  R8Unorm,
  R8G8Unorm,
  R8G8B8A8Unorm,
  R8G8B8A8Srgb,
  B8G8R8A8Unorm,
  B8G8R8A8Srgb,
  A2B10G10R10Unorm,
  R16G16B16A16Float,
  R32G32B32A32Float,
  R11G11B10Float,
  D16Unorm,
  D24UnormS8Uint,
  D32Float,
  D32FloatS8Uint,
  Bc1Unorm,
  Bc1Srgb,
  Bc3Unorm,
  Bc3Srgb,
  Bc5Unorm,
  Bc7Unorm,
  Bc7Srgb,
};

/// @brief Name of a `TextureFormat` enumerator.
/// @param format Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(
    TextureFormat format) noexcept {
  switch (format) {
    using enum TextureFormat;
    case Undefined:
      return "Undefined";
    case R8Unorm:
      return "R8Unorm";
    case R8G8Unorm:
      return "R8G8Unorm";
    case R8G8B8A8Unorm:
      return "R8G8B8A8Unorm";
    case R8G8B8A8Srgb:
      return "R8G8B8A8Srgb";
    case B8G8R8A8Unorm:
      return "B8G8R8A8Unorm";
    case B8G8R8A8Srgb:
      return "B8G8R8A8Srgb";
    case A2B10G10R10Unorm:
      return "A2B10G10R10Unorm";
    case R16G16B16A16Float:
      return "R16G16B16A16Float";
    case R32G32B32A32Float:
      return "R32G32B32A32Float";
    case R11G11B10Float:
      return "R11G11B10Float";
    case D16Unorm:
      return "D16Unorm";
    case D24UnormS8Uint:
      return "D24UnormS8Uint";
    case D32Float:
      return "D32Float";
    case D32FloatS8Uint:
      return "D32FloatS8Uint";
    case Bc1Unorm:
      return "Bc1Unorm";
    case Bc1Srgb:
      return "Bc1Srgb";
    case Bc3Unorm:
      return "Bc3Unorm";
    case Bc3Srgb:
      return "Bc3Srgb";
    case Bc5Unorm:
      return "Bc5Unorm";
    case Bc7Unorm:
      return "Bc7Unorm";
    case Bc7Srgb:
      return "Bc7Srgb";
  }
  return "Unknown";
}

/// @brief Filter-time usage mask for `SupportsFormat`.
enum class FormatUsage : uint32_t {
  Sample = 1U << 0U,
  Filter = 1U << 1U,
  Storage = 1U << 2U,
  Color = 1U << 3U,
  Depth = 1U << 4U,
  Blend = 1U << 5U,
  Copy = 1U << 6U,
  Resolve = 1U << 7U,
  Atomic = 1U << 8U,
};

/// @brief Name of a `FormatUsage` enumerator.
/// @param usage Exact enumerator. Combined masks return `"Flags"`.
/// @return Enumerator name, `"Flags"`, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(FormatUsage usage) noexcept {
  switch (usage) {
    using enum FormatUsage;
    case Sample:
      return "Sample";
    case Filter:
      return "Filter";
    case Storage:
      return "Storage";
    case Color:
      return "Color";
    case Depth:
      return "Depth";
    case Blend:
      return "Blend";
    case Copy:
      return "Copy";
    case Resolve:
      return "Resolve";
    case Atomic:
      return "Atomic";
  }
  return "Flags";
}

[[nodiscard]] constexpr FormatUsage operator|(FormatUsage lhs,
                                              FormatUsage rhs) noexcept {
  return static_cast<FormatUsage>(std::to_underlying(lhs) |
                                  std::to_underlying(rhs));
}

[[nodiscard]] constexpr FormatUsage operator&(FormatUsage lhs,
                                              FormatUsage rhs) noexcept {
  return static_cast<FormatUsage>(std::to_underlying(lhs) &
                                  std::to_underlying(rhs));
}

[[nodiscard]] constexpr FormatUsage operator~(FormatUsage value) noexcept {
  return static_cast<FormatUsage>(~std::to_underlying(value));
}

constexpr FormatUsage& operator|=(FormatUsage& lhs, FormatUsage rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr FormatUsage& operator&=(FormatUsage& lhs, FormatUsage rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

/// @brief True if every bit in `bits` is set in `set`.
/// @param set Mask to test
/// @param bits Required bits
/// @return `true` if `(set & bits) == bits`
[[nodiscard]] constexpr bool HasAll(FormatUsage set,
                                    FormatUsage bits) noexcept {
  return (set & bits) == bits;
}

}  // namespace aperture
