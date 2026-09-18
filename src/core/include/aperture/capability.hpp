#pragma once

#include <cstdint>
#include <string_view>
#include <utility>

namespace aperture {

/// @brief Optional device features negotiated at `CreateDevice`.
/// @details Reported on `Adapter::capabilities`.
enum class Capability : uint16_t {
  None = 0,
  MeshShading = 1U << 0U,
  RayTracingPipeline = 1U << 1U,
  RayQuery = 1U << 2U,
  SeparateBlend = 1U << 3U,
  HostImageCopy = 1U << 4U,
  UnifiedImageLayouts = 1U << 5U,
  DeviceGeneratedCommands = 1U << 6U,
  AsyncCompute = 1U << 7U,
  AsyncCopy = 1U << 8U,
  PresentFromCompute = 1U << 9U,
  CooperativeMatrix = 1U << 10U,
  FramebufferFetch = 1U << 11U,
  BufferInt64Atomics = 1U << 12U,
  SplitBarriers = 1U << 13U,
};

/// @brief Name of a `Capability` enumerator.
/// @param caps Exact enumerator or `None`. Combined masks return `"Flags"`.
/// @return Enumerator name, `"None"`, `"Flags"`, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(Capability caps) noexcept {
  switch (caps) {
    using enum Capability;
    case None:
      return "None";
    case MeshShading:
      return "MeshShading";
    case RayTracingPipeline:
      return "RayTracingPipeline";
    case RayQuery:
      return "RayQuery";
    case SeparateBlend:
      return "SeparateBlend";
    case HostImageCopy:
      return "HostImageCopy";
    case UnifiedImageLayouts:
      return "UnifiedImageLayouts";
    case DeviceGeneratedCommands:
      return "DeviceGeneratedCommands";
    case AsyncCompute:
      return "AsyncCompute";
    case AsyncCopy:
      return "AsyncCopy";
    case PresentFromCompute:
      return "PresentFromCompute";
    case CooperativeMatrix:
      return "CooperativeMatrix";
    case FramebufferFetch:
      return "FramebufferFetch";
    case BufferInt64Atomics:
      return "BufferInt64Atomics";
    case SplitBarriers:
      return "SplitBarriers";
  }
  return "Flags";
}

[[nodiscard]] constexpr Capability operator|(Capability lhs,
                                             Capability rhs) noexcept {
  return static_cast<Capability>(std::to_underlying(lhs) |
                                 std::to_underlying(rhs));
}

[[nodiscard]] constexpr Capability operator&(Capability lhs,
                                             Capability rhs) noexcept {
  return static_cast<Capability>(std::to_underlying(lhs) &
                                 std::to_underlying(rhs));
}

[[nodiscard]] constexpr Capability operator~(Capability value) noexcept {
  return static_cast<Capability>(~std::to_underlying(value));
}

constexpr Capability& operator|=(Capability& lhs, Capability rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr Capability& operator&=(Capability& lhs, Capability rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

/// @brief True if every bit in `bits` is set in `set`.
/// @param set Mask to test
/// @param bits Required bits
/// @return `true` if `(set & bits) == bits`
[[nodiscard]] constexpr bool HasAll(Capability set, Capability bits) noexcept {
  return (set & bits) == bits;
}

}  // namespace aperture
