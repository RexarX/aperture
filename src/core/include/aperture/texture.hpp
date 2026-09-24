#pragma once

#include <cstdint>
#include <string_view>
#include <utility>

namespace aperture {

/// @brief Shader-visible texture heap index. Slot 0 is the reserved null view.
enum class TextureSlot : uint32_t { Null = 0 };

/// @brief Name of a `TextureSlot` enumerator.
/// @param slot Exact enumerator. Issued indices other than `Null` return
/// `"Slot"`.
/// @return Enumerator name, or `"Slot"`
[[nodiscard]] constexpr std::string_view ToString(TextureSlot slot) noexcept {
  switch (slot) {
    using enum TextureSlot;
    case Null:
      return "Null";
  }
  return "Slot";
}

/// @brief Shader-visible sampler heap index.
/// @details `Null` through `ShadowCompare` are device-lifetime samplers.
/// `AllocSamplerSlot` starts at `FirstUser`.
enum class SamplerSlot : uint32_t {
  Null = 0,
  PointClamp,
  PointWrap,
  LinearClamp,
  LinearWrap,
  ShadowCompare,
  FirstUser,
};

/// @brief Name of a `SamplerSlot` enumerator.
/// @param slot Exact enumerator. Indices at or above `FirstUser` return
/// `"Slot"`.
/// @return Enumerator name, or `"Slot"`
[[nodiscard]] constexpr std::string_view ToString(SamplerSlot slot) noexcept;

/// @brief Placement requirement for a texture's backing memory.
/// @details `heap` is an opaque tag consumed by the image-heap `MallocGpu`
/// overload. `align` divides `Info(device).texture_heap_alignment`.
struct SizeAlign {
  uint64_t size = 0;
  uint64_t align = 0;
  uint32_t heap = 0;
};

/// @brief CPU handle for a texture object. `Invalid` is never a live texture.
enum class Texture : uint8_t { Invalid = 0 };

/// @brief Creation / barrier usage mask for a texture.
enum class TextureUsage : uint32_t {
  Sampled = 1U << 0U,
  Storage = 1U << 1U,
  Color = 1U << 2U,
  DepthStencil = 1U << 3U,
  CopySrc = 1U << 4U,
  CopyDst = 1U << 5U,
};

/// @brief Name of a `TextureUsage` enumerator.
/// @param usage Exact enumerator. Combined masks return `"Flags"`.
/// @return Enumerator name, `"Flags"`, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(TextureUsage usage) noexcept {
  switch (usage) {
    using enum TextureUsage;
    case Sampled:
      return "Sampled";
    case Storage:
      return "Storage";
    case Color:
      return "Color";
    case DepthStencil:
      return "DepthStencil";
    case CopySrc:
      return "CopySrc";
    case CopyDst:
      return "CopyDst";
  }
  return "Flags";
}

[[nodiscard]] constexpr TextureUsage operator|(TextureUsage lhs,
                                               TextureUsage rhs) noexcept {
  return static_cast<TextureUsage>(std::to_underlying(lhs) |
                                   std::to_underlying(rhs));
}

[[nodiscard]] constexpr TextureUsage operator&(TextureUsage lhs,
                                               TextureUsage rhs) noexcept {
  return static_cast<TextureUsage>(std::to_underlying(lhs) &
                                   std::to_underlying(rhs));
}

[[nodiscard]] constexpr TextureUsage operator~(TextureUsage value) noexcept {
  return static_cast<TextureUsage>(~std::to_underlying(value));
}

constexpr TextureUsage& operator|=(TextureUsage& lhs,
                                   TextureUsage rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr TextureUsage& operator&=(TextureUsage& lhs,
                                   TextureUsage rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

[[nodiscard]] constexpr bool HasAll(TextureUsage set,
                                    TextureUsage bits) noexcept {
  return (set & bits) == bits;
}

/// @brief Magnification / minification filter.
enum class Filter : uint8_t { Nearest, Linear };

/// @brief Name of a `Filter` enumerator.
/// @param filter Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(Filter filter) noexcept {
  switch (filter) {
    using enum Filter;
    case Nearest:
      return "Nearest";
    case Linear:
      return "Linear";
  }
  return "Unknown";
}

/// @brief Sampler addressing mode.
enum class Address : uint8_t { Repeat, Mirror, Clamp, Border };

/// @brief Name of an `Address` enumerator.
/// @param address Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(Address address) noexcept {
  switch (address) {
    using enum Address;
    case Repeat:
      return "Repeat";
    case Mirror:
      return "Mirror";
    case Clamp:
      return "Clamp";
    case Border:
      return "Border";
  }
  return "Unknown";
}

/// @brief Depth / stencil compare function.
enum class Compare : uint8_t {
  Never,
  Less,
  Equal,
  LessEqual,
  Greater,
  NotEqual,
  GreaterEqual,
  Always
};

/// @brief Name of a `Compare` enumerator.
/// @param compare Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(Compare compare) noexcept {
  switch (compare) {
    using enum Compare;
    case Never:
      return "Never";
    case Less:
      return "Less";
    case Equal:
      return "Equal";
    case LessEqual:
      return "LessEqual";
    case Greater:
      return "Greater";
    case NotEqual:
      return "NotEqual";
    case GreaterEqual:
      return "GreaterEqual";
    case Always:
      return "Always";
  }
  return "Unknown";
}

/// @brief Sampler reduction mode.
enum class Reduction : uint8_t { Average, Min, Max };

/// @brief Name of a `Reduction` enumerator.
/// @param reduction Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(
    Reduction reduction) noexcept {
  switch (reduction) {
    using enum Reduction;
    case Average:
      return "Average";
    case Min:
      return "Min";
    case Max:
      return "Max";
  }
  return "Unknown";
}

/// @brief Attachment load operation at the start of a rendering scope.
enum class Load : uint8_t { Load, Clear, DontCare };

/// @brief Name of a `Load` enumerator.
/// @param load Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(Load load) noexcept {
  switch (load) {
    using enum Load;
    case Load:
      return "Load";
    case Clear:
      return "Clear";
    case DontCare:
      return "DontCare";
  }
  return "Unknown";
}

inline constexpr std::string_view ToString(SamplerSlot slot) noexcept {
  switch (slot) {
    using enum SamplerSlot;
    case Null:
      return "Null";
    case PointClamp:
      return "PointClamp";
    case PointWrap:
      return "PointWrap";
    case LinearClamp:
      return "LinearClamp";
    case LinearWrap:
      return "LinearWrap";
    case ShadowCompare:
      return "ShadowCompare";
    case FirstUser:
      return "FirstUser";
  }
  return "Slot";
}

}  // namespace aperture
