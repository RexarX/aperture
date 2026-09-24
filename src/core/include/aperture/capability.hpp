#pragma once

#include <aperture/utils/flags.hpp>

#include <cstdint>
#include <string_view>

namespace aperture {

/// @brief Optional device features negotiated at `CreateDevice`.
/// @details Reported on `Adapter::capabilities`. Stored as `uint64_t` so new
/// bits can be added without changing the type.
enum class Capability : uint64_t {
  None = 0ULL,
  MeshShading = 1ULL << 0ULL,
  RayTracingPipeline = 1ULL << 1ULL,
  RayQuery = 1ULL << 2ULL,
  SeparateBlend = 1ULL << 3ULL,
  HostImageCopy = 1ULL << 4ULL,
  UnifiedImageLayouts = 1ULL << 5ULL,
  DeviceGeneratedCommands = 1ULL << 6ULL,
  AsyncCompute = 1ULL << 7ULL,
  AsyncCopy = 1ULL << 8ULL,
  PresentFromCompute = 1ULL << 9ULL,
  CooperativeMatrix = 1ULL << 10ULL,
  FramebufferFetch = 1ULL << 11ULL,
  BufferInt64Atomics = 1ULL << 12ULL,
  SplitBarriers = 1ULL << 13ULL,
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

template <>
inline constexpr bool IS_FLAGS<Capability> = true;

}  // namespace aperture
