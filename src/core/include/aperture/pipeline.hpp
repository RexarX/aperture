#pragma once

#include <aperture/types.hpp>

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace aperture {

/// @brief Compiled pipeline object. `Invalid` is never a live pipeline.
enum class Pipeline : uint8_t { Invalid = 0 };

/// @brief One compiled module in a native backend encoding.
/// @details `code` is borrowed. Aperture does not copy, take ownership, or
/// read a filesystem path.
struct ShaderBlob {
  std::span<const std::byte> code;
  ShaderFormat format = ShaderFormat::Invalid;
};

/// @brief Depth/stencil state object. `Invalid` is never a live state.
enum class DepthStencilState : uint8_t { Invalid = 0 };

/// @brief Blend state object. `Invalid` is never a live state.
enum class BlendState : uint8_t { Invalid = 0 };

/// @brief Raster polygon fill mode.
enum class PolygonMode : uint8_t { Fill, Line };

/// @brief Name of a `PolygonMode` enumerator.
/// @param mode Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(PolygonMode mode) noexcept {
  switch (mode) {
    using enum PolygonMode;
    case Fill:
      return "Fill";
    case Line:
      return "Line";
  }
  return "Unknown";
}

/// @brief Triangle culling mode.
enum class Cull : uint8_t { None, Front, Back };

/// @brief Name of a `Cull` enumerator.
/// @param cull Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(Cull cull) noexcept {
  switch (cull) {
    using enum Cull;
    case None:
      return "None";
    case Front:
      return "Front";
    case Back:
      return "Back";
  }
  return "Unknown";
}

/// @brief Triangle winding that is considered front-facing.
enum class FrontFace : uint8_t { Ccw, Cw };

/// @brief Name of a `FrontFace` enumerator.
/// @param face Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(FrontFace face) noexcept {
  switch (face) {
    using enum FrontFace;
    case Ccw:
      return "Ccw";
    case Cw:
      return "Cw";
  }
  return "Unknown";
}

/// @brief Behavior when a pipeline is missing from the cache at draw time.
/// @details `SkipDraw` drops the draw. Querying which draws were skipped is
/// not part of v1.
enum class PipelinePolicy : uint8_t { FailOnMiss, SkipDraw, Block };

/// @brief Name of a `PipelinePolicy` enumerator.
/// @param policy Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(
    PipelinePolicy policy) noexcept {
  switch (policy) {
    using enum PipelinePolicy;
    case FailOnMiss:
      return "FailOnMiss";
    case SkipDraw:
      return "SkipDraw";
    case Block:
      return "Block";
  }
  return "Unknown";
}

}  // namespace aperture
