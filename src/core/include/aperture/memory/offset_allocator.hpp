#pragma once

#include <aperture/assert.hpp>
#include <aperture/platform.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace aperture {

/// @brief Fast hard-realtime O(1) offset allocator (TLSF-style binning).
/// @details Based on Sebastian Aaltonen's OffsetAllocator (MIT, 2023). 256
/// bins with an 8-bit float distribution (3-bit mantissa + 5-bit exponent) and
/// a two-level bitfield. Next free bin is found with two LZCNT/TZCNT ops.
/// Metadata lives outside the storage, so this can suballocate GPU heaps,
/// buffers, or arrays. Returns an offset to the first element of a contiguous
/// range. TLSF here means Two-Level Segregated Fit, not thread-local storage.
/// @warning Not internally synchronized. Concurrent `Allocate` / `Free` /
/// `Reset` on the same instance is a data race.
class APERTURE_API OffsetAllocator {
public:
  static constexpr uint32_t TOP_BIN_COUNT = 32;
  static constexpr uint32_t BINS_PER_LEAF = 8;
  static constexpr uint32_t TOP_BIN_SHIFT = 3;
  static constexpr uint32_t LEAF_BIN_MASK = 0x7;
  static constexpr uint32_t LEAF_BIN_COUNT = TOP_BIN_COUNT * BINS_PER_LEAF;
  static constexpr uint32_t DEFAULT_MAX_ALLOCS = 128U * 1024U;

  /// @brief One contiguous range inside the managed storage.
  struct Allocation {
    static constexpr uint32_t NO_SPACE = 0xFFFFFFFFU;

    uint32_t offset = NO_SPACE;
    /// Internal node index. Needed to free.
    uint32_t metadata = NO_SPACE;

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

  /// @brief Coarse free-space query.
  struct StorageReport {
    uint32_t free_bytes = 0;
    uint32_t largest_free_region = 0;
  };

  /// @brief Per-bin free-region counts. 256 bins.
  struct StorageReportFull {
    struct Region {
      uint32_t size = 0;
      uint32_t count = 0;
    };

    std::array<Region, LEAF_BIN_COUNT> free_regions = {};
  };

  /// @brief Takes ownership of a `[0, size)` range to suballocate.
  /// @param size Byte count of the managed storage. Must be non-zero
  /// @param max_allocs Node pool size (live allocations plus free fragments)
  /// @warning Asserts if `size` or `max_allocs` is 0.
  explicit OffsetAllocator(uint32_t size,
                           uint32_t max_allocs = DEFAULT_MAX_ALLOCS) noexcept;
  OffsetAllocator(const OffsetAllocator&) = delete;
  OffsetAllocator(OffsetAllocator&& other) noexcept { MoveFrom(&other); }
  ~OffsetAllocator() noexcept = default;

  OffsetAllocator& operator=(const OffsetAllocator&) = delete;
  OffsetAllocator& operator=(OffsetAllocator&& other) noexcept;

  /// @brief Drops every allocation and restores a single free region.
  void Reset() noexcept;

  /// @brief Allocates `size` contiguous bytes.
  /// @param size Byte count. Must be non-zero
  /// @return Offset and metadata, or a false `Allocation` if out of space
  /// @warning Asserts if `size` is 0.
  [[nodiscard]] Allocation Allocate(uint32_t size) noexcept;

  /// @brief Returns `allocation` to the pool and coalesces neighbors.
  /// @param allocation Result of `Allocate`
  /// @warning Asserts if `allocation` is not a live allocation from this
  /// allocator.
  void Free(Allocation allocation) noexcept;

  /// @brief Byte count of `allocation`.
  /// @param allocation Result of `Allocate`
  /// @return Size passed to `Allocate`, or 0 if `allocation` is empty
  [[nodiscard]] uint32_t AllocationSize(Allocation allocation) const noexcept;

  /// @brief Checks for live allocations.
  /// @return `true` if every byte is free (no live allocations), `false`
  /// otherwise
  [[nodiscard]] bool Empty() const noexcept { return used_count_ == 0; }

  /// @brief Gets free bytes and a lower bound on the largest free region.
  /// @return Storage report
  [[nodiscard]] StorageReport Report() const noexcept;

  /// @brief Gets per-bin free-region size and count.
  /// @return Full storage report
  [[nodiscard]] StorageReportFull ReportFull() const noexcept;

  /// @brief Gets the managed storage size in bytes.
  /// @return Managed storage size in bytes
  [[nodiscard]] uint32_t Size() const noexcept { return size_; }

  /// @brief Gets currently free bytes (may be fragmented).
  /// @return Currently free bytes
  [[nodiscard]] uint32_t FreeBytes() const noexcept { return free_bytes_; }

private:
  /// Used flag lives in the bitset after the node array.
  struct Node {
    static constexpr uint32_t UNUSED = 0xFFFFFFFFU;

    uint32_t offset = 0;
    uint32_t size = 0;
    uint32_t bin_prev = UNUSED;
    uint32_t bin_next = UNUSED;
    uint32_t neighbor_prev = UNUSED;
    uint32_t neighbor_next = UNUSED;
  };

  [[nodiscard]] uint32_t InsertNodeIntoBin(uint32_t size,
                                           uint32_t data_offset) noexcept;
  void RemoveNodeFromBin(uint32_t node_index) noexcept;
  void MoveFrom(OffsetAllocator* other) noexcept;

  [[nodiscard]] Node* Nodes() noexcept {
    return reinterpret_cast<Node*>(storage_.get());
  }

  [[nodiscard]] const Node* Nodes() const noexcept {
    return reinterpret_cast<const Node*>(storage_.get());
  }

  [[nodiscard]] uint8_t* UsedBits() noexcept {
    return reinterpret_cast<uint8_t*>(storage_.get() +
                                      (sizeof(Node) * max_allocs_));
  }

  [[nodiscard]] const uint8_t* UsedBits() const noexcept {
    return reinterpret_cast<const uint8_t*>(storage_.get() +
                                            (sizeof(Node) * max_allocs_));
  }

  [[nodiscard]] bool IsUsed(uint32_t index) const noexcept {
    return (UsedBits()[index >> 3U] &
            static_cast<uint8_t>(1U << (index & 7U))) != 0;
  }

  void SetUsed(uint32_t index, bool used) noexcept;

  std::unique_ptr<std::byte[]> storage_;
  std::unique_ptr<uint32_t[]> free_nodes_;
  std::array<uint32_t, LEAF_BIN_COUNT> bin_indices_ = {};
  uint32_t size_ = 0;
  uint32_t max_allocs_ = 0;
  uint32_t free_bytes_ = 0;
  uint32_t used_bins_top_ = 0;
  uint32_t free_offset_ = 0;
  uint32_t used_count_ = 0;
  std::array<uint8_t, TOP_BIN_COUNT> used_bins_ = {};
};

inline OffsetAllocator::OffsetAllocator(uint32_t size,
                                        uint32_t max_allocs) noexcept
    : size_(size), max_allocs_(max_allocs) {
  APERTURE_ASSERT(size != 0);
  APERTURE_ASSERT(max_allocs != 0);
  Reset();
}

inline OffsetAllocator& OffsetAllocator::operator=(
    OffsetAllocator&& other) noexcept {
  if (this == &other) [[unlikely]] {
    return *this;
  }

  storage_.reset();
  free_nodes_.reset();
  MoveFrom(&other);
  return *this;
}

inline uint32_t OffsetAllocator::AllocationSize(
    Allocation allocation) const noexcept {
  if (!allocation || storage_ == nullptr) [[unlikely]] {
    return 0;
  }
  return Nodes()[allocation.metadata].size;
}

inline void OffsetAllocator::SetUsed(uint32_t index, bool used) noexcept {
  uint8_t& byte = UsedBits()[index >> 3U];
  const uint8_t mask = static_cast<uint8_t>(1U << (index & 7U));
  const bool was_used = (byte & mask) != 0;
  if (used) {
    byte = static_cast<uint8_t>(byte | mask);
    if (!was_used) {
      ++used_count_;
    }
  } else {
    byte = static_cast<uint8_t>(byte & static_cast<uint8_t>(~mask));
    if (was_used) {
      --used_count_;
    }
  }
}

}  // namespace aperture
