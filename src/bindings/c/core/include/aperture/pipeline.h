#ifndef APERTURE_PIPELINE_H
#define APERTURE_PIPELINE_H

#include <aperture/types.h>

#include <stddef.h>
#include <stdint.h>

/// @brief Pointer-sized CPU handle for a compiled pipeline. Invalid is `NULL`.
typedef struct AperturePipelineImpl* AperturePipeline;

/// @brief One compiled module in a native backend encoding.
typedef struct ApertureShaderBlob {
  const void* code;
  size_t size;
  const char* entry;
  size_t entry_size;
  ApertureShaderFormat format;
} ApertureShaderBlob;

/// @brief Pointer-sized CPU handle for depth/stencil state. Invalid is `NULL`.
typedef struct ApertureDepthStencilStateImpl* ApertureDepthStencilState;

/// @brief Pointer-sized CPU handle for blend state. Invalid is `NULL`.
typedef struct ApertureBlendStateImpl* ApertureBlendState;

/// @brief Raster polygon fill mode.
typedef uint8_t AperturePolygonMode;

enum {
  APERTURE_POLYGON_MODE_FILL = 0U,
  APERTURE_POLYGON_MODE_LINE,
};

/// @brief Triangle culling mode.
typedef uint8_t ApertureCull;

enum {
  APERTURE_CULL_NONE = 0U,
  APERTURE_CULL_FRONT,
  APERTURE_CULL_BACK,
};

/// @brief Triangle winding that is considered front-facing.
typedef uint8_t ApertureFrontFace;

enum {
  APERTURE_FRONT_FACE_CCW = 0U,
  APERTURE_FRONT_FACE_CW,
};

/// @brief Behavior when a pipeline is missing from the cache at draw time.
/// @details `SKIP_DRAW` drops the draw. Querying which draws were skipped is
/// not part of v1.
typedef uint8_t AperturePipelinePolicy;

enum {
  APERTURE_PIPELINE_POLICY_FAIL_ON_MISS = 0U,
  APERTURE_PIPELINE_POLICY_SKIP_DRAW,
  APERTURE_PIPELINE_POLICY_BLOCK,
};

/// @brief Name of an `AperturePipelinePolicy` enumerator.
/// @param policy Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
static inline const char* aperture_pipeline_policy_to_string(
    AperturePipelinePolicy policy) {
  switch (policy) {
    case APERTURE_PIPELINE_POLICY_FAIL_ON_MISS:
      return "FailOnMiss";
    case APERTURE_PIPELINE_POLICY_SKIP_DRAW:
      return "SkipDraw";
    case APERTURE_PIPELINE_POLICY_BLOCK:
      return "Block";
    default:
      return "Unknown";
  }
}

#endif
