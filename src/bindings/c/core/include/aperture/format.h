#ifndef APERTURE_FORMAT_H
#define APERTURE_FORMAT_H

#include <stdbool.h>
#include <stdint.h>

/// @brief Closed texture format set. Values may grow; existing values stay
/// stable.
typedef uint8_t ApertureTextureFormat;

enum {
  APERTURE_TEXTURE_FORMAT_UNDEFINED = 0U,
  APERTURE_TEXTURE_FORMAT_R8_UNORM,
  APERTURE_TEXTURE_FORMAT_R8G8_UNORM,
  APERTURE_TEXTURE_FORMAT_R8G8B8A8_UNORM,
  APERTURE_TEXTURE_FORMAT_R8G8B8A8_SRGB,
  APERTURE_TEXTURE_FORMAT_B8G8R8A8_UNORM,
  APERTURE_TEXTURE_FORMAT_B8G8R8A8_SRGB,
  APERTURE_TEXTURE_FORMAT_A2B10G10R10_UNORM,
  APERTURE_TEXTURE_FORMAT_R16G16B16A16_FLOAT,
  APERTURE_TEXTURE_FORMAT_R32G32B32A32_FLOAT,
  APERTURE_TEXTURE_FORMAT_R11G11B10_FLOAT,
  APERTURE_TEXTURE_FORMAT_D16_UNORM,
  APERTURE_TEXTURE_FORMAT_D24_UNORM_S8_UINT,
  APERTURE_TEXTURE_FORMAT_D32_FLOAT,
  APERTURE_TEXTURE_FORMAT_D32_FLOAT_S8_UINT,
  APERTURE_TEXTURE_FORMAT_BC1_UNORM,
  APERTURE_TEXTURE_FORMAT_BC1_SRGB,
  APERTURE_TEXTURE_FORMAT_BC3_UNORM,
  APERTURE_TEXTURE_FORMAT_BC3_SRGB,
  APERTURE_TEXTURE_FORMAT_BC5_UNORM,
  APERTURE_TEXTURE_FORMAT_BC7_UNORM,
  APERTURE_TEXTURE_FORMAT_BC7_SRGB,
};

/// @brief Name of an `ApertureTextureFormat` enumerator.
/// @param format Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
static inline const char* aperture_texture_format_to_string(
    ApertureTextureFormat format) {
  switch (format) {
    case APERTURE_TEXTURE_FORMAT_UNDEFINED:
      return "Undefined";
    case APERTURE_TEXTURE_FORMAT_R8_UNORM:
      return "R8Unorm";
    case APERTURE_TEXTURE_FORMAT_R8G8_UNORM:
      return "R8G8Unorm";
    case APERTURE_TEXTURE_FORMAT_R8G8B8A8_UNORM:
      return "R8G8B8A8Unorm";
    case APERTURE_TEXTURE_FORMAT_R8G8B8A8_SRGB:
      return "R8G8B8A8Srgb";
    case APERTURE_TEXTURE_FORMAT_B8G8R8A8_UNORM:
      return "B8G8R8A8Unorm";
    case APERTURE_TEXTURE_FORMAT_B8G8R8A8_SRGB:
      return "B8G8R8A8Srgb";
    case APERTURE_TEXTURE_FORMAT_A2B10G10R10_UNORM:
      return "A2B10G10R10Unorm";
    case APERTURE_TEXTURE_FORMAT_R16G16B16A16_FLOAT:
      return "R16G16B16A16Float";
    case APERTURE_TEXTURE_FORMAT_R32G32B32A32_FLOAT:
      return "R32G32B32A32Float";
    case APERTURE_TEXTURE_FORMAT_R11G11B10_FLOAT:
      return "R11G11B10Float";
    case APERTURE_TEXTURE_FORMAT_D16_UNORM:
      return "D16Unorm";
    case APERTURE_TEXTURE_FORMAT_D24_UNORM_S8_UINT:
      return "D24UnormS8Uint";
    case APERTURE_TEXTURE_FORMAT_D32_FLOAT:
      return "D32Float";
    case APERTURE_TEXTURE_FORMAT_D32_FLOAT_S8_UINT:
      return "D32FloatS8Uint";
    case APERTURE_TEXTURE_FORMAT_BC1_UNORM:
      return "Bc1Unorm";
    case APERTURE_TEXTURE_FORMAT_BC1_SRGB:
      return "Bc1Srgb";
    case APERTURE_TEXTURE_FORMAT_BC3_UNORM:
      return "Bc3Unorm";
    case APERTURE_TEXTURE_FORMAT_BC3_SRGB:
      return "Bc3Srgb";
    case APERTURE_TEXTURE_FORMAT_BC5_UNORM:
      return "Bc5Unorm";
    case APERTURE_TEXTURE_FORMAT_BC7_UNORM:
      return "Bc7Unorm";
    case APERTURE_TEXTURE_FORMAT_BC7_SRGB:
      return "Bc7Srgb";
    default:
      return "Unknown";
  }
}

/// @brief Filter-time usage mask for `aperture_supports_format`.
typedef uint32_t ApertureFormatUsage;

enum {
  APERTURE_FORMAT_USAGE_SAMPLE = 1U << 0U,
  APERTURE_FORMAT_USAGE_FILTER = 1U << 1U,
  APERTURE_FORMAT_USAGE_STORAGE = 1U << 2U,
  APERTURE_FORMAT_USAGE_COLOR = 1U << 3U,
  APERTURE_FORMAT_USAGE_DEPTH = 1U << 4U,
  APERTURE_FORMAT_USAGE_BLEND = 1U << 5U,
  APERTURE_FORMAT_USAGE_COPY = 1U << 6U,
  APERTURE_FORMAT_USAGE_RESOLVE = 1U << 7U,
  APERTURE_FORMAT_USAGE_ATOMIC = 1U << 8U,
};

/// @brief Name of an `ApertureFormatUsage` enumerator.
/// @param usage Exact enumerator. Combined masks return `"Flags"`.
/// @return Enumerator name, or `"Flags"`
static inline const char* aperture_format_usage_to_string(
    ApertureFormatUsage usage) {
  switch (usage) {
    case APERTURE_FORMAT_USAGE_SAMPLE:
      return "Sample";
    case APERTURE_FORMAT_USAGE_FILTER:
      return "Filter";
    case APERTURE_FORMAT_USAGE_STORAGE:
      return "Storage";
    case APERTURE_FORMAT_USAGE_COLOR:
      return "Color";
    case APERTURE_FORMAT_USAGE_DEPTH:
      return "Depth";
    case APERTURE_FORMAT_USAGE_BLEND:
      return "Blend";
    case APERTURE_FORMAT_USAGE_COPY:
      return "Copy";
    case APERTURE_FORMAT_USAGE_RESOLVE:
      return "Resolve";
    case APERTURE_FORMAT_USAGE_ATOMIC:
      return "Atomic";
    default:
      return "Flags";
  }
}

/// @brief True if every bit in `bits` is set in `set`.
/// @param set Mask to test
/// @param bits Required bits
/// @return `true` if `(set & bits) == bits`
static inline bool aperture_format_usage_has_all(ApertureFormatUsage set,
                                                 ApertureFormatUsage bits) {
  return (ApertureFormatUsage)(set & bits) == bits;
}

#endif
