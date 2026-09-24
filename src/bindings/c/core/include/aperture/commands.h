#ifndef APERTURE_COMMANDS_H
#define APERTURE_COMMANDS_H

#include <aperture/platform.h>

#include <stdbool.h>
#include <stdint.h>

APERTURE_C_BEGIN

/// @brief Memory class of an allocation.
typedef uint8_t ApertureMemory;

enum {
  APERTURE_MEMORY_DEFAULT = 0U,
  APERTURE_MEMORY_READBACK,
};

/// @brief Name of an `ApertureMemory` enumerator.
/// @param memory Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
static inline const char* aperture_memory_to_string(ApertureMemory memory) {
  switch (memory) {
    case APERTURE_MEMORY_DEFAULT:
      return "Default";
    case APERTURE_MEMORY_READBACK:
      return "Readback";
    default:
      return "Unknown";
  }
}

/// @brief Placement flags for malloc.
typedef uint32_t ApertureMallocFlags;

enum {
  APERTURE_MALLOC_FLAGS_NONE = 0U,
  APERTURE_MALLOC_FLAGS_DEDICATED = 1U << 0U,
};

/// @brief Name of an `ApertureMallocFlags` enumerator.
/// @param flags Exact enumerator or `NONE`. Combined masks return `"Flags"`.
/// @return Enumerator name, `"Flags"`, or `"Unknown"`
static inline const char* aperture_malloc_flags_to_string(
    ApertureMallocFlags flags) {
  switch (flags) {
    case APERTURE_MALLOC_FLAGS_NONE:
      return "None";
    case APERTURE_MALLOC_FLAGS_DEDICATED:
      return "Dedicated";
    default:
      return "Flags";
  }
}

/// @brief Pipeline stages for barriers and timeline waits.
typedef uint32_t ApertureStage;

enum {
  APERTURE_STAGE_NONE = 0U,
  APERTURE_STAGE_HOST = 1U << 0U,
  APERTURE_STAGE_COPY = 1U << 1U,
  APERTURE_STAGE_COMPUTE = 1U << 2U,
  APERTURE_STAGE_INDIRECT = 1U << 3U,
  APERTURE_STAGE_VERTEX = 1U << 4U,
  APERTURE_STAGE_PIXEL = 1U << 5U,
  APERTURE_STAGE_MESH = 1U << 6U,
  APERTURE_STAGE_TASK = 1U << 7U,
  APERTURE_STAGE_RASTER_COLOR_OUT = 1U << 8U,
  APERTURE_STAGE_RASTER_DEPTH_OUT = 1U << 9U,
  APERTURE_STAGE_RAY = 1U << 10U,
  APERTURE_STAGE_ALL = 0x7FFFFFFFU,
};

/// @brief Extra hazard bits that are not implied by `ApertureStage` alone.
typedef uint32_t ApertureHazard;

enum {
  APERTURE_HAZARD_NONE = 0U,
  APERTURE_HAZARD_DRAW_ARGUMENTS = 1U << 0U,
  APERTURE_HAZARD_DESCRIPTORS = 1U << 1U,
  APERTURE_HAZARD_DEPTH_STENCIL = 1U << 2U,
  APERTURE_HAZARD_INDEX_BUFFER = 1U << 3U,
};

/// @brief True if every bit in `bits` is set in `set`.
/// @param set Mask to test
/// @param bits Required bits
/// @return `true` if `(set & bits) == bits`
static inline bool aperture_stage_has_all(ApertureStage set,
                                          ApertureStage bits) {
  return (ApertureStage)(set & bits) == bits;
}

/// @brief True if every bit in `bits` is set in `set`.
/// @param set Mask to test
/// @param bits Required bits
/// @return `true` if `(set & bits) == bits`
static inline bool aperture_hazard_has_all(ApertureHazard set,
                                           ApertureHazard bits) {
  return (ApertureHazard)(set & bits) == bits;
}

/// @brief Pool that allocates command buffers. Invalid is `NULL`.
/// @brief Parameters for command-pool creation.
/// @details `max_timestamps` is the number of timestamp writes one buffer from
/// the pool may record. Zero disables timestamps.
typedef struct ApertureCommandPoolDesc {
  uint32_t max_timestamps;
} ApertureCommandPoolDesc;

typedef struct ApertureCommandPoolImpl* ApertureCommandPool;

/// @brief One-shot recording token. Consumed by submit. Invalid is `NULL`.
typedef struct ApertureCommandBufferImpl* ApertureCommandBuffer;

APERTURE_C_END

#endif
