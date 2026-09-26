#pragma once

#include <aperture/assert.hpp>
#include <aperture/platform.hpp>
#include <aperture/types.hpp>
#include <aperture/utils/bit.hpp>

#include <atomic>
#include <cstdint>

namespace aperture {

/// @brief Wait-free bump allocator over a `[0, size)` range.
/// @details Metadata lives outside the storage, so this can suballocate CPU
/// arrays, GPU heaps, buffers, or mapped DualPtr ranges. Returns an offset to
/// the first element of a contiguous range. Concurrent `Allocate` calls are a
/// single `fetch_add` and return disjoint ranges. Reservations are rounded up
/// to `DEFAULT_ALIGN` so the cursor stays aligned without a CAS. `Reset`,
/// move, and destruction require exclusive access.
class APERTURE_API BumpAllocator {
public:
  static constexpr uint64_t DEFAULT_ALIGN = 16;

  /// @brief One contiguous range inside the managed storage.
  struct Allocation {
    static constexpr auto NO_SPACE = ~uint64_t{0};

    uint64_t offset = NO_SPACE;
    uint64_t size = 0;

    [[nodiscard]] constexpr bool operator==(const Allocation&) const noexcept =
        default;
    [[nodiscard]] constexpr bool operator!=(const Allocation&) const noexcept =
        default;

    /// @brief Checks if this is a live allocation.
    /// @return `true` if this is a live allocation, `false` otherwise
    [[nodiscard]] constexpr explicit operator bool() const noexcept {
      return offset != NO_SPACE;
    }
  };

  /// @brief Takes ownership of a `[0, size)` range to suballocate.
  /// @param size Byte count of the managed storage. Must be non-zero
  /// @warning Asserts if `size` is 0.
  explicit BumpAllocator(uint64_t size) noexcept : size_(size) {
    APERTURE_ASSERT(size != 0);
  }
  BumpAllocator(const BumpAllocator&) = delete;
  BumpAllocator(BumpAllocator&& other) noexcept { MoveFrom(&other); }
  ~BumpAllocator() noexcept = default;

  BumpAllocator& operator=(const BumpAllocator&) = delete;
  BumpAllocator& operator=(BumpAllocator&& other) noexcept;

  /// @brief Rewinds the cursor to empty. Live allocations become invalid.
  /// @warning Must not run concurrently with `Allocate`.
  void Reset() noexcept { offset_.store(0, std::memory_order_release); }

  /// @brief Allocates `size` contiguous bytes. Wait-free with other
  /// `Allocate` calls.
  /// @param size Byte count. Must be non-zero
  /// @param align Alignment in bytes. Power of two, at most `DEFAULT_ALIGN`
  /// @return Offset and size, or a false `Allocation` if out of space
  /// @warning Asserts if `size` is 0, or if `align` is not a power of two in
  /// `[1, DEFAULT_ALIGN]`.
  /// @note A failed allocation still advances the cursor by the rounded
  /// reservation. Call `Reset` before reuse.
  [[nodiscard]] Allocation Allocate(uint64_t size,
                                    uint64_t align = DEFAULT_ALIGN) noexcept;

  /// @brief Typed bump allocate: `count * sizeof(T)` bytes at `alignof(T)`.
  /// @tparam T Element type. `alignof(T)` must be <= `DEFAULT_ALIGN`
  /// @param count Element count (default: 1)
  /// @return Offset and size, or a false `Allocation` if out of space
  /// @warning Asserts if `count` is 0.
  template <typename T>
  [[nodiscard]] Allocation Allocate(uint64_t count = 1) noexcept;

  /// @brief Checks for live allocations.
  /// @return `true` if the cursor is at zero, `false` otherwise
  [[nodiscard]] bool Empty() const noexcept { return Used() == 0; }

  /// @brief Gets the managed storage size in bytes.
  /// @return Managed storage size in bytes
  [[nodiscard]] uint64_t Size() const noexcept { return size_; }

  /// @brief Gets bytes consumed by the cursor, including alignment padding.
  /// @return Used bytes
  [[nodiscard]] uint64_t Used() const noexcept {
    return offset_.load(std::memory_order_relaxed);
  }

  /// @brief Gets remaining bytes after the cursor.
  /// @details A failed `Allocate` still advances the cursor, so `Used()` can
  /// exceed `Size()`. This saturates at 0 in that case.
  /// @return Free bytes, or 0 when the cursor is past the end
  [[nodiscard]] uint64_t FreeBytes() const noexcept;

private:
  void MoveFrom(BumpAllocator* other) noexcept;

  uint64_t size_ = 0;
  std::atomic<uint64_t> offset_{0};
};

inline uint64_t BumpAllocator::FreeBytes() const noexcept {
  const uint64_t used = Used();
  return used >= size_ ? 0 : size_ - used;
}

inline BumpAllocator& BumpAllocator::operator=(BumpAllocator&& other) noexcept {
  if (this == &other) [[unlikely]] {
    return *this;
  }

  MoveFrom(&other);
  return *this;
}

/// @brief Forms a `DualRange` from a bump allocation into `base`.
/// @param base Mapped storage the allocator was sized against
/// @param allocation Offset and size from `BumpAllocator::Allocate`
/// @return Host and device addresses at `base + offset`
/// @warning Asserts if `allocation` is not live.
[[nodiscard]] inline DualRange Slice(
    DualPtr<std::byte> base, BumpAllocator::Allocation allocation) noexcept {
  APERTURE_ASSERT(allocation);
  return {.ptr = base + allocation.offset, .size = allocation.size};
}

template <typename T>
inline auto BumpAllocator::Allocate(uint64_t count) noexcept -> Allocation {
  APERTURE_ASSERT(count != 0);
  static_assert(alignof(T) <= DEFAULT_ALIGN);
  if (utils::MulOverflows(static_cast<uint64_t>(sizeof(T)), count))
      [[unlikely]] {
    return {};
  }
  return Allocate(count * sizeof(T), alignof(T));
}

}  // namespace aperture
