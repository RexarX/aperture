#ifndef APERTURE_CAPABILITY_H
#define APERTURE_CAPABILITY_H

#include <stdbool.h>
#include <stdint.h>

/// @brief Optional device features negotiated at device creation.
/// @details Stored as `uint64_t` so new bits can be added without changing
/// the type. Combined masks are valid; `aperture_capability_to_string`
/// returns `"Flags"` for them.
typedef uint64_t ApertureCapability;

enum {
  APERTURE_CAPABILITY_NONE = 0ULL,
  APERTURE_CAPABILITY_MESH_SHADING = 1ULL << 0ULL,
  APERTURE_CAPABILITY_RAY_TRACING_PIPELINE = 1ULL << 1ULL,
  APERTURE_CAPABILITY_RAY_QUERY = 1ULL << 2ULL,
  APERTURE_CAPABILITY_SEPARATE_BLEND = 1ULL << 3ULL,
  APERTURE_CAPABILITY_HOST_IMAGE_COPY = 1ULL << 4ULL,
  APERTURE_CAPABILITY_UNIFIED_IMAGE_LAYOUTS = 1ULL << 5ULL,
  APERTURE_CAPABILITY_DEVICE_GENERATED_COMMANDS = 1ULL << 6ULL,
  APERTURE_CAPABILITY_ASYNC_COMPUTE = 1ULL << 7ULL,
  APERTURE_CAPABILITY_ASYNC_COPY = 1ULL << 8ULL,
  APERTURE_CAPABILITY_PRESENT_FROM_COMPUTE = 1ULL << 9ULL,
  APERTURE_CAPABILITY_COOPERATIVE_MATRIX = 1ULL << 10ULL,
  APERTURE_CAPABILITY_FRAMEBUFFER_FETCH = 1ULL << 11ULL,
  APERTURE_CAPABILITY_BUFFER_INT64_ATOMICS = 1ULL << 12ULL,
  APERTURE_CAPABILITY_SPLIT_BARRIERS = 1ULL << 13ULL,
};

/// @brief Name of an `ApertureCapability` enumerator.
/// @param caps Exact enumerator or `NONE`. Combined masks return `"Flags"`.
/// @return Enumerator name, `"None"`, or `"Flags"`
static inline const char* aperture_capability_to_string(
    ApertureCapability caps) {
  switch (caps) {
    case APERTURE_CAPABILITY_NONE:
      return "None";
    case APERTURE_CAPABILITY_MESH_SHADING:
      return "MeshShading";
    case APERTURE_CAPABILITY_RAY_TRACING_PIPELINE:
      return "RayTracingPipeline";
    case APERTURE_CAPABILITY_RAY_QUERY:
      return "RayQuery";
    case APERTURE_CAPABILITY_SEPARATE_BLEND:
      return "SeparateBlend";
    case APERTURE_CAPABILITY_HOST_IMAGE_COPY:
      return "HostImageCopy";
    case APERTURE_CAPABILITY_UNIFIED_IMAGE_LAYOUTS:
      return "UnifiedImageLayouts";
    case APERTURE_CAPABILITY_DEVICE_GENERATED_COMMANDS:
      return "DeviceGeneratedCommands";
    case APERTURE_CAPABILITY_ASYNC_COMPUTE:
      return "AsyncCompute";
    case APERTURE_CAPABILITY_ASYNC_COPY:
      return "AsyncCopy";
    case APERTURE_CAPABILITY_PRESENT_FROM_COMPUTE:
      return "PresentFromCompute";
    case APERTURE_CAPABILITY_COOPERATIVE_MATRIX:
      return "CooperativeMatrix";
    case APERTURE_CAPABILITY_FRAMEBUFFER_FETCH:
      return "FramebufferFetch";
    case APERTURE_CAPABILITY_BUFFER_INT64_ATOMICS:
      return "BufferInt64Atomics";
    case APERTURE_CAPABILITY_SPLIT_BARRIERS:
      return "SplitBarriers";
    default:
      return "Flags";
  }
}

/// @brief True if every bit in `bits` is set in `set`.
/// @param set Mask to test
/// @param bits Required bits
/// @return `true` if `(set & bits) == bits`
static inline bool aperture_capability_has_all(ApertureCapability set,
                                               ApertureCapability bits) {
  return (ApertureCapability)(set & bits) == bits;
}

#endif
