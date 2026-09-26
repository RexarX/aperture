#pragma once

#include <aperture/platform.hpp>
#include <aperture/queue.hpp>
#include <aperture/result.hpp>

#include <cstdint>

namespace aperture {

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

/// @brief Allocates a command pool for `queue`.
/// @param queue Queue the pool's buffers will be submitted to
/// @param desc Pool parameters. `max_timestamps == 0` disables timestamps
/// @return The pool, or a recoverable `Error`
/// @warning Asserts if `queue` is null.
/// @warning `Begin` and `Reset` on one pool are externally synchronized.
[[nodiscard]] APERTURE_API auto CreateCommandPool(
    Queue queue, const CommandPoolDesc& desc = {}) noexcept
    -> Result<CommandPool>;

/// @brief Recycles every buffer allocated from `pool`.
/// @param pool Pool to reset. Null is a no-op
/// @warning Does not wait on the GPU. Buffers from `pool` must not be pending.
/// @warning Asserts if a buffer from `pool` is still recording.
APERTURE_API void Reset(CommandPool pool) noexcept;

/// @brief Destroys a command pool. Immediate; does not wait on the GPU.
/// @param pool Pool to destroy. Null is a no-op
/// @warning Asserts if a buffer from `pool` is still recording.
APERTURE_API void Destroy(CommandPool pool) noexcept;

}  // namespace aperture
