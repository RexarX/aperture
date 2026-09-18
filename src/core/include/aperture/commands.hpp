#pragma once

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

/// @brief Behavior when an arena runs out of space.
enum class ArenaOverflow : uint8_t { Fail, Grow };

/// @brief Name of an `ArenaOverflow` enumerator.
/// @param overflow Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(
    ArenaOverflow overflow) noexcept {
  switch (overflow) {
    using enum ArenaOverflow;
    case Fail:
      return "Fail";
    case Grow:
      return "Grow";
  }
  return "Unknown";
}

/// @brief Index buffer element type.
enum class IndexType : uint8_t { U16, U32 };

/// @brief Name of an `IndexType` enumerator.
/// @param type Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(IndexType type) noexcept {
  switch (type) {
    using enum IndexType;
    case U16:
      return "U16";
    case U32:
      return "U32";
  }
  return "Unknown";
}

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

[[nodiscard]] constexpr Stage operator|(Stage lhs, Stage rhs) noexcept {
  return static_cast<Stage>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

[[nodiscard]] constexpr Stage operator&(Stage lhs, Stage rhs) noexcept {
  return static_cast<Stage>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

[[nodiscard]] constexpr Stage operator~(Stage value) noexcept {
  return static_cast<Stage>(~std::to_underlying(value));
}

constexpr Stage& operator|=(Stage& lhs, Stage rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr Stage& operator&=(Stage& lhs, Stage rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

/// @brief True if every bit in `bits` is set in `set`.
/// @param set Mask to test
/// @param bits Required bits
/// @return `true` if `(set & bits) == bits`
[[nodiscard]] constexpr bool HasAll(Stage set, Stage bits) noexcept {
  return (set & bits) == bits;
}

/// @brief Extra hazard bits that are not implied by `Stage` alone.
enum class Hazard : uint8_t {
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

[[nodiscard]] constexpr Hazard operator|(Hazard lhs, Hazard rhs) noexcept {
  return static_cast<Hazard>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

[[nodiscard]] constexpr Hazard operator&(Hazard lhs, Hazard rhs) noexcept {
  return static_cast<Hazard>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

[[nodiscard]] constexpr Hazard operator~(Hazard value) noexcept {
  return static_cast<Hazard>(~std::to_underlying(value));
}

constexpr Hazard& operator|=(Hazard& lhs, Hazard rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr Hazard& operator&=(Hazard& lhs, Hazard rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

/// @brief True if every bit in `bits` is set in `set`.
/// @param set Mask to test
/// @param bits Required bits
/// @return `true` if `(set & bits) == bits`
[[nodiscard]] constexpr bool HasAll(Hazard set, Hazard bits) noexcept {
  return (set & bits) == bits;
}

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

/// @brief Bump allocator for transient GPU memory. Pointer-sized CPU handle.
struct Arena {
  void* ptr = nullptr;

  /// @brief True if this handle is non-null.
  /// @return `true` when `ptr != nullptr`
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return ptr != nullptr;
  }

  [[nodiscard]] constexpr bool operator==(const Arena&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const Arena&) const noexcept =
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
