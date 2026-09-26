#pragma once

#include <aperture/command/pool.hpp>
#include <aperture/platform.hpp>
#include <aperture/result.hpp>
#include <aperture/types.hpp>
#include <aperture/utils/flags.hpp>

#include <cstdint>
#include <string_view>

namespace aperture {

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

/// @brief One-shot recording token. Consumed by `Submit`.
/// @details Pointer-sized CPU handle. `Submit` sets `ptr` to null.
struct CommandBuffer {
  void* ptr = nullptr;

  /// @brief True if this token still owns a recording.
  /// @return `true` when `ptr != nullptr`
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return ptr != nullptr;
  }

  [[nodiscard]] constexpr bool operator==(const CommandBuffer&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const CommandBuffer&) const noexcept =
      default;
};

/// @brief Begins one one-shot command buffer from `pool`.
/// @param pool Pool that allocates the buffer
/// @return The recording token, or `Error::OutOfMemory`
/// @warning Asserts if `pool` is null.
/// @warning Pools do not stall-and-grow.
[[nodiscard]] APERTURE_API auto Begin(CommandPool pool) noexcept
    -> Result<CommandBuffer>;

/// @brief Records a stage-mask memory barrier.
/// @param buffer Recording token
/// @param src Source stages. The scope includes every earlier stage
/// @param dst Destination stages. The scope includes every later stage
/// @param hazards Extra cache flushes. `None` is the common case
/// @warning Asserts in next cases:
/// - If `buffer` is null
/// - If `buffer` is not recording
/// - If `src` is `None`
/// - If `dst` is `None`
APERTURE_API void Barrier(CommandBuffer* buffer, Stage src, Stage dst,
                          Hazard hazards = Hazard::None) noexcept;

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
/// - If the pool queue is missing from an allocation's `QueueUsage`
APERTURE_API void Copy(CommandBuffer* buffer, GpuRange dst,
                       GpuRange src) noexcept;

}  // namespace aperture
