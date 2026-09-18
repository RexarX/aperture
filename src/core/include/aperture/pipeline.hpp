#pragma once

#include <cstdint>
#include <string_view>

namespace aperture {

/// @brief Compiled pipeline object. `Invalid` is never a live pipeline.
enum class Pipeline : uint8_t { Invalid = 0 };

/// @brief Name of a `Pipeline` enumerator.
/// @param pipeline Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(Pipeline pipeline) noexcept {
  switch (pipeline) {
    using enum Pipeline;
    case Invalid:
      return "Invalid";
  }
  return "Unknown";
}

/// @brief Linked shader program. `Invalid` is never a live program.
enum class Program : uint8_t { Invalid = 0 };

/// @brief Name of a `Program` enumerator.
/// @param program Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(Program program) noexcept {
  switch (program) {
    using enum Program;
    case Invalid:
      return "Invalid";
  }
  return "Unknown";
}

/// @brief Depth/stencil state object. `Invalid` is never a live state.
enum class DepthStencilState : uint8_t { Invalid = 0 };

/// @brief Name of a `DepthStencilState` enumerator.
/// @param state Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(
    DepthStencilState state) noexcept {
  switch (state) {
    using enum DepthStencilState;
    case Invalid:
      return "Invalid";
  }
  return "Unknown";
}

/// @brief Blend state object. `Invalid` is never a live state.
enum class BlendState : uint8_t { Invalid = 0 };

/// @brief Name of a `BlendState` enumerator.
/// @param state Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(BlendState state) noexcept {
  switch (state) {
    using enum BlendState;
    case Invalid:
      return "Invalid";
  }
  return "Unknown";
}

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

/// @brief Matrix memory layout in shader / C++ mirrors.
enum class MatrixLayout : uint8_t { ColumnMajor, RowMajor };

/// @brief Name of a `MatrixLayout` enumerator.
/// @param layout Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(
    MatrixLayout layout) noexcept {
  using enum MatrixLayout;
  switch (layout) {
    case ColumnMajor:
      return "ColumnMajor";
    case RowMajor:
      return "RowMajor";
  }
  return "Unknown";
}

/// @brief Behavior when a pipeline is missing from the cache at draw time.
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
