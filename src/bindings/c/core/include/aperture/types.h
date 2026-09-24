#ifndef APERTURE_TYPES_H
#define APERTURE_TYPES_H

#include <stdbool.h>
#include <stdint.h>

/// @brief Native graphics API selected at instance creation.
typedef uint8_t ApertureBackend;

enum {
  APERTURE_BACKEND_VULKAN = 0U,
  APERTURE_BACKEND_D3D12,
  APERTURE_BACKEND_METAL,
};

/// @brief Name of an `ApertureBackend` enumerator.
/// @param backend Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
static inline const char* aperture_backend_to_string(ApertureBackend backend) {
  switch (backend) {
    case APERTURE_BACKEND_VULKAN:
      return "Vulkan";
    case APERTURE_BACKEND_D3D12:
      return "D3D12";
    case APERTURE_BACKEND_METAL:
      return "Metal";
    default:
      return "Unknown";
  }
}

/// @brief Native compiled-shader encoding of a shader blob.
typedef uint8_t ApertureShaderFormat;

enum {
  APERTURE_SHADER_FORMAT_INVALID = 0U,
  APERTURE_SHADER_FORMAT_SPIRV,
  APERTURE_SHADER_FORMAT_DXIL,
  APERTURE_SHADER_FORMAT_METALLIB,
};

/// @brief SPIR-V magic (little-endian).
#define APERTURE_SPIRV_MAGIC 0x07230203U

/// @brief SPIR-V magic with byte-swapped endianness.
#define APERTURE_SPIRV_MAGIC_SWAPPED 0x03022307U

/// @brief DXBC container magic (`'DXBC'`).
#define APERTURE_DXBC_MAGIC 0x43425844U

/// @brief Metal library magic (`'MTLB'`).
#define APERTURE_MTLB_MAGIC 0x424C544DU

/// @brief Name of an `ApertureShaderFormat` enumerator.
/// @param format Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
static inline const char* aperture_shader_format_to_string(
    ApertureShaderFormat format) {
  switch (format) {
    case APERTURE_SHADER_FORMAT_INVALID:
      return "Invalid";
    case APERTURE_SHADER_FORMAT_SPIRV:
      return "Spirv";
    case APERTURE_SHADER_FORMAT_DXIL:
      return "Dxil";
    case APERTURE_SHADER_FORMAT_METALLIB:
      return "MetalLib";
    default:
      return "Unknown";
  }
}

/// @brief Native blob encoding consumed by `backend`.
/// @param backend Instance / device backend
/// @return Format that program creation accepts on that backend
static inline ApertureShaderFormat aperture_native_shader_format(
    ApertureBackend backend) {
  switch (backend) {
    case APERTURE_BACKEND_VULKAN:
      return APERTURE_SHADER_FORMAT_SPIRV;
    case APERTURE_BACKEND_D3D12:
      return APERTURE_SHADER_FORMAT_DXIL;
    case APERTURE_BACKEND_METAL:
      return APERTURE_SHADER_FORMAT_METALLIB;
    default:
      return APERTURE_SHADER_FORMAT_INVALID;
  }
}

/// @brief True when `format` is the encoding `backend` consumes.
/// @param format Declared blob encoding
/// @param backend Instance / device backend
/// @return `true` if `format` is non-invalid and matches `backend`
static inline bool aperture_shader_format_compatible(
    ApertureShaderFormat format, ApertureBackend backend) {
  return format != APERTURE_SHADER_FORMAT_INVALID &&
         format == aperture_native_shader_format(backend);
}

/// @brief 64-bit device address.
typedef struct ApertureGpuPtr {
  uint64_t addr;
} ApertureGpuPtr;

/// @brief Texel-block counts for copy-queue texture copies.
/// @details A zero component means that axis must cover the whole mip.
typedef struct ApertureCopyGranularity {
  uint32_t x;
  uint32_t y;
  uint32_t z;
} ApertureCopyGranularity;

/// @brief Timestamp tick period and which queue kinds can write them.
/// @details `period_ns` is zero when timestamps are unsupported.
typedef struct ApertureTimestampSupport {
  float period_ns;
  bool graphics;
  bool compute;
  bool copy;
} ApertureTimestampSupport;

/// @brief Host-mapped allocation plus its device address.
typedef struct ApertureDualPtr {
  void* host;
  ApertureGpuPtr device;
} ApertureDualPtr;

/// @brief Device address plus byte count.
typedef struct ApertureGpuRange {
  ApertureGpuPtr gpu;
  uint64_t size;
} ApertureGpuRange;

/// @brief Host-mapped pointer, device address, and byte count.
typedef struct ApertureDualRange {
  ApertureDualPtr ptr;
  uint64_t size;
} ApertureDualRange;

/// @brief `count * elem_size` bytes starting at `ptr`.
/// @param ptr Device address of the first element
/// @param count Element count
/// @param elem_size Bytes per element
/// @return Range covering that span
static inline ApertureGpuRange aperture_gpu_range_from(ApertureGpuPtr ptr,
                                                       uint64_t count,
                                                       uint64_t elem_size) {
  const ApertureGpuRange range = {
      .gpu = ptr,
      .size = count * elem_size,
  };
  return range;
}

/// @brief `count * elem_size` bytes starting at `ptr.device`.
/// @param ptr Host-mapped allocation
/// @param count Element count
/// @param elem_size Bytes per element
/// @return Device-only range covering that span
static inline ApertureGpuRange aperture_gpu_range_from_dual(
    ApertureDualPtr ptr, uint64_t count, uint64_t elem_size) {
  return aperture_gpu_range_from(ptr.device, count, elem_size);
}

/// @brief `count * elem_size` bytes starting at `ptr`.
/// @param ptr Host-mapped allocation
/// @param count Element count
/// @param elem_size Bytes per element
/// @return Dual range covering that span
static inline ApertureDualRange aperture_dual_range_from(ApertureDualPtr ptr,
                                                         uint64_t count,
                                                         uint64_t elem_size) {
  const ApertureDualRange range = {
      .ptr = ptr,
      .size = count * elem_size,
  };
  return range;
}

/// @brief Device-only view of a dual range.
/// @param range Host-mapped range
/// @return Device address plus byte count
static inline ApertureGpuRange aperture_dual_range_device(
    ApertureDualRange range) {
  const ApertureGpuRange gpu = {
      .gpu = range.ptr.device,
      .size = range.size,
  };
  return gpu;
}

#endif
