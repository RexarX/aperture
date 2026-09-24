#pragma once

#include <aperture/types.hpp>

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace aperture {

/// @brief Pointer-sized CPU handle for a compiled pipeline.
struct Pipeline {
  void* ptr = nullptr;

  /// @brief True if this handle is non-null.
  /// @return `true` when `ptr != nullptr`
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return ptr != nullptr;
  }

  [[nodiscard]] constexpr bool operator==(const Pipeline&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const Pipeline&) const noexcept =
      default;
};

/// @brief One compiled module in a native backend encoding.
/// @details `code` is borrowed. Aperture does not copy, take ownership, or
/// read a filesystem path.
struct ShaderBlob {
  std::span<const std::byte> code;
  /// Entry point name. Empty is `Error::Invalid` at pipeline create.
  std::string_view entry;
  ShaderFormat format = ShaderFormat::Invalid;
};

/// @brief Pointer-sized CPU handle for depth/stencil state.
struct DepthStencilState {
  void* ptr = nullptr;

  /// @brief True if this handle is non-null.
  /// @return `true` when `ptr != nullptr`
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return ptr != nullptr;
  }

  [[nodiscard]] constexpr bool operator==(
      const DepthStencilState&) const noexcept = default;
  [[nodiscard]] constexpr bool operator!=(
      const DepthStencilState&) const noexcept = default;
};

/// @brief Pointer-sized CPU handle for blend state.
struct BlendState {
  void* ptr = nullptr;

  /// @brief True if this handle is non-null.
  /// @return `true` when `ptr != nullptr`
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return ptr != nullptr;
  }

  [[nodiscard]] constexpr bool operator==(const BlendState&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const BlendState&) const noexcept =
      default;
};

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
