#ifndef APERTURE_COMMAND_BUFFER_H
#define APERTURE_COMMAND_BUFFER_H

#include <aperture/command/pool.h>
#include <aperture/platform.h>
#include <aperture/result.h>
#include <aperture/types.h>

#include <stdbool.h>
#include <stdint.h>

APERTURE_C_BEGIN

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

/// @brief One-shot recording token. Consumed by submit. Invalid is `NULL`.
typedef struct ApertureCommandBufferImpl* ApertureCommandBuffer;

/// @brief Begins one one-shot command buffer from `pool`.
/// @param pool Pool that allocates the buffer
/// @param out Receives the recording token on success
/// @return The begin error, or `APERTURE_ERROR_OK`
/// @warning Asserts in next cases:
/// - If `pool` is null
/// - If `out` is null
APERTURE_C_API ApertureError aperture_begin(
    ApertureCommandPool pool, ApertureCommandBuffer* out) APERTURE_C_NOEXCEPT;

/// @brief Records a stage-mask memory barrier.
/// @param buffer Recording token
/// @param src Source stages
/// @param dst Destination stages
/// @param hazards Extra hazard bits
/// @warning Asserts in next cases:
/// - If `buffer` is null
/// - If `buffer` is not recording
/// - If `src` is `APERTURE_STAGE_NONE`
/// - If `dst` is `APERTURE_STAGE_NONE`
APERTURE_C_API void aperture_barrier(
    ApertureCommandBuffer buffer, ApertureStage src, ApertureStage dst,
    ApertureHazard hazards) APERTURE_C_NOEXCEPT;

/// @brief Records a buffer-to-buffer copy.
/// @param buffer Recording token
/// @param dst Destination bytes
/// @param src Source bytes
/// @warning Asserts in next cases:
/// - If `buffer` is null
/// - If `buffer` is not recording
/// - If either range is empty
/// - If either address is unknown
/// - If a range extends past its allocation
/// - If the ranges differ in size
/// - If the ranges overlap in one buffer
/// - If the pool queue is missing from an allocation's queue usage
APERTURE_C_API void aperture_copy(ApertureCommandBuffer buffer,
                                  ApertureGpuRange dst,
                                  ApertureGpuRange src) APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
