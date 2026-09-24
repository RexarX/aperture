#pragma once

#include <cstddef>
#include <cstdint>

namespace aperture::vk {

/// @brief Maximum size of a Vulkan descriptor blob, in bytes.
inline constexpr uint32_t MAX_DESCRIPTOR_BYTES = 64;

/// @brief Opaque descriptor bytes for a user-managed Vulkan heap.
/// @details `size` is the heap stride. Core `Store` encodes directly into the
/// heap; this blob is for callers that build a descriptor once and copy it.
struct Descriptor {
  alignas(8) std::byte data[MAX_DESCRIPTOR_BYTES] = {};
  uint32_t size = 0;
};

}  // namespace aperture::vk
