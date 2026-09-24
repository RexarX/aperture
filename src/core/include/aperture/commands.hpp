#pragma once

#include <aperture/utils/flags.hpp>

#include <cstdint>
#include <string_view>
#include <utility>

namespace aperture {

/// @brief Memory class of an allocation.
enum class Memory : uint8_t { Default, Readback };

/// @brief Name of a `Memory` enumerator.
/// @param memory Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(Memory memory) noexcept {
  switch (memory) {
    using enum Memory;
    case Default:
      return "Default";
    case Readback:
      return "Readback";
  }
  return "Unknown";
}

/// @brief Placement flags for `Malloc` and `MallocGpu`.
enum class MallocFlags : uint32_t {
  None = 0,
  Dedicated = 1U << 0U,
};

/// @brief Name of a `MallocFlags` enumerator.
/// @param flags Exact enumerator or `None`. Combined masks return `"Flags"`.
/// @return Enumerator name, `"Flags"`, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(MallocFlags flags) noexcept {
  switch (flags) {
    using enum MallocFlags;
    case None:
      return "None";
    case Dedicated:
      return "Dedicated";
  }
  return "Flags";
}

template <>
inline constexpr bool IS_FLAGS<MallocFlags> = true;

/// @brief Pipeline stages for barriers and timeline waits.
enum class Stage : uint32_t {
  None = 0,
  Host = 1U << 0U,
  Copy = 1U << 1U,
  Compute = 1U << 2U,
  Indirect = 1U << 3U,
  Vertex = 1U << 4U,
  Pixel = 1U << 5U,
  Mesh = 1U << 6U,
  Task = 1U << 7U,
  RasterColorOut = 1U << 8U,
  RasterDepthOut = 1U << 9U,
  Ray = 1U << 10U,
  All = 0x7FFFFFFFU,
};

/// @brief Name of a `Stage` enumerator.
/// @param stage Exact enumerator, `None`, or `All`. Combined masks return
/// `"Flags"`.
/// @return Enumerator name, `"Flags"`, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(Stage stage) noexcept {
  switch (stage) {
    using enum Stage;
    case None:
      return "None";
    case Host:
      return "Host";
    case Copy:
      return "Copy";
    case Compute:
      return "Compute";
    case Indirect:
      return "Indirect";
    case Vertex:
      return "Vertex";
    case Pixel:
      return "Pixel";
    case Mesh:
      return "Mesh";
    case Task:
      return "Task";
    case RasterColorOut:
      return "RasterColorOut";
    case RasterDepthOut:
      return "RasterDepthOut";
    case Ray:
      return "Ray";
    case All:
      return "All";
  }
  return "Flags";
}

template <>
inline constexpr bool IS_FLAGS<Stage> = true;

/// @brief Extra hazard bits that are not implied by `Stage` alone.
enum class Hazard : uint32_t {
  None = 0,
  DrawArguments = 1U << 0U,
  Descriptors = 1U << 1U,
  DepthStencil = 1U << 2U,
  IndexBuffer = 1U << 3U,
};

/// @brief Name of a `Hazard` enumerator.
/// @param hazard Exact enumerator or `None`. Combined masks return `"Flags"`.
/// @return Enumerator name, `"Flags"`, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(Hazard hazard) noexcept {
  switch (hazard) {
    using enum Hazard;
    case None:
      return "None";
    case DrawArguments:
      return "DrawArguments";
    case Descriptors:
      return "Descriptors";
    case DepthStencil:
      return "DepthStencil";
    case IndexBuffer:
      return "IndexBuffer";
  }
  return "Flags";
}

template <>
inline constexpr bool IS_FLAGS<Hazard> = true;

/// @brief Parameters for `CreateCommandPool`.
/// @details `max_timestamps` is the number of `WriteTimestamp` calls one
/// buffer from the pool may record. Zero disables timestamps.
struct CommandPoolDesc {
  uint32_t max_timestamps = 0;
};

/// @brief Pool that allocates command buffers. Pointer-sized CPU handle.
struct CommandPool {
  void* ptr = nullptr;

  /// @brief True if this handle is non-null.
  /// @return `true` when `ptr != nullptr`
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return ptr != nullptr;
  }

  [[nodiscard]] constexpr bool operator==(const CommandPool&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const CommandPool&) const noexcept =
      default;
};

/// @brief One-shot recording token. Consumed by Submit.
/// @details Move-only: copying would double-submit or double-free the token.
struct CommandBuffer {
  constexpr CommandBuffer() noexcept = default;
  CommandBuffer(const CommandBuffer&) = delete;
  constexpr CommandBuffer(CommandBuffer&& other) noexcept
      : ptr(std::exchange(other.ptr, nullptr)) {}
  constexpr ~CommandBuffer() noexcept = default;

  CommandBuffer& operator=(const CommandBuffer&) = delete;
  constexpr CommandBuffer& operator=(CommandBuffer&& other) noexcept {
    if (this != &other) [[likely]] {
      ptr = std::exchange(other.ptr, nullptr);
    }
    return *this;
  }

  void* ptr = nullptr;

  [[nodiscard]] constexpr bool operator==(const CommandBuffer&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const CommandBuffer&) const noexcept =
      default;
};

}  // namespace aperture
