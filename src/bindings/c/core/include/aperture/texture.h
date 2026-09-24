#ifndef APERTURE_TEXTURE_H
#define APERTURE_TEXTURE_H

#include <aperture/platform.h>

#include <stdint.h>

/// @brief Shader-visible texture heap index. Slot 0 is the reserved null view.
typedef uint32_t ApertureTextureSlot;

enum { APERTURE_TEXTURE_SLOT_NULL = 0U };

/// @brief Shader-visible sampler heap index.
/// @details `NULL` through `SHADOW_COMPARE` are device-lifetime samplers.
/// Allocation starts at `FIRST_USER`.
typedef uint32_t ApertureSamplerSlot;

enum {
  APERTURE_SAMPLER_SLOT_NULL = 0U,
  APERTURE_SAMPLER_SLOT_POINT_CLAMP,
  APERTURE_SAMPLER_SLOT_POINT_WRAP,
  APERTURE_SAMPLER_SLOT_LINEAR_CLAMP,
  APERTURE_SAMPLER_SLOT_LINEAR_WRAP,
  APERTURE_SAMPLER_SLOT_SHADOW_COMPARE,
  APERTURE_SAMPLER_SLOT_FIRST_USER,
};

/// @brief Placement requirement for a texture's backing memory.
typedef struct ApertureSizeAlign {
  uint64_t size;
  uint64_t align;
  uint32_t heap;
} ApertureSizeAlign;

/// @brief CPU handle for a texture object. `INVALID` is never a live texture.
typedef struct ApertureTexture {
  uint8_t id;
} ApertureTexture;

enum { APERTURE_TEXTURE_INVALID = 0U };

/// @brief Creation / barrier usage mask for a texture.
typedef uint32_t ApertureTextureUsage;

enum {
  APERTURE_TEXTURE_USAGE_SAMPLED = 1U << 0U,
  APERTURE_TEXTURE_USAGE_STORAGE = 1U << 1U,
  APERTURE_TEXTURE_USAGE_COLOR = 1U << 2U,
  APERTURE_TEXTURE_USAGE_DEPTH_STENCIL = 1U << 3U,
  APERTURE_TEXTURE_USAGE_COPY_SRC = 1U << 4U,
  APERTURE_TEXTURE_USAGE_COPY_DST = 1U << 5U,
};

/// @brief Magnification / minification filter.
typedef uint8_t ApertureFilter;

enum {
  APERTURE_FILTER_NEAREST = 0U,
  APERTURE_FILTER_LINEAR,
};

/// @brief Sampler addressing mode.
typedef uint8_t ApertureAddress;

enum {
  APERTURE_ADDRESS_REPEAT = 0U,
  APERTURE_ADDRESS_MIRROR,
  APERTURE_ADDRESS_CLAMP,
  APERTURE_ADDRESS_BORDER,
};

/// @brief Depth / stencil compare function.
typedef uint8_t ApertureCompare;

enum {
  APERTURE_COMPARE_NEVER = 0U,
  APERTURE_COMPARE_LESS,
  APERTURE_COMPARE_EQUAL,
  APERTURE_COMPARE_LESS_EQUAL,
  APERTURE_COMPARE_GREATER,
  APERTURE_COMPARE_NOT_EQUAL,
  APERTURE_COMPARE_GREATER_EQUAL,
  APERTURE_COMPARE_ALWAYS,
};

/// @brief Sampler reduction mode.
typedef uint8_t ApertureReduction;

enum {
  APERTURE_REDUCTION_AVERAGE = 0U,
  APERTURE_REDUCTION_MIN,
  APERTURE_REDUCTION_MAX,
};

/// @brief Attachment load operation at the start of a rendering scope.
typedef uint8_t ApertureLoad;

enum {
  APERTURE_LOAD_LOAD = 0U,
  APERTURE_LOAD_CLEAR,
  APERTURE_LOAD_DONT_CARE,
};

#endif
