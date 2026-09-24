#include <pch.hpp>

#include <aperture/memory/offset_allocator.hpp>

#include <aperture/assert.hpp>

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <new>
#include <utility>

namespace aperture {

namespace {

[[nodiscard]] constexpr uint32_t FindLowestSetBitAfter(
    uint32_t bit_mask, uint32_t start_bit) noexcept {
  if (start_bit >= 32) {
    return OffsetAllocator::Allocation::NO_SPACE;
  }
  const uint32_t bits_after = bit_mask & (~0U << start_bit);
  if (bits_after == 0) {
    return OffsetAllocator::Allocation::NO_SPACE;
  }
  return std::countr_zero(bits_after);
}

constexpr uint32_t MANTISSA_BITS = 3;
constexpr uint32_t MANTISSA_VALUE = 1U << MANTISSA_BITS;
constexpr uint32_t MANTISSA_MASK = MANTISSA_VALUE - 1;

/// Bin index from size, rounding up so the bin is large enough to hold `size`.
[[nodiscard]] constexpr uint32_t UintToFloatRoundUp(uint32_t size) noexcept {
  if (size < MANTISSA_VALUE) {
    return size;
  }

  const uint32_t leading_zeros = std::countl_zero(size);
  const uint32_t highest_set_bit = 31U - leading_zeros;
  const uint32_t mantissa_start_bit = highest_set_bit - MANTISSA_BITS;
  uint32_t mantissa = (size >> mantissa_start_bit) & MANTISSA_MASK;
  const uint32_t low_bits_mask = (1U << mantissa_start_bit) - 1U;
  if ((size & low_bits_mask) != 0) {
    ++mantissa;
  }
  const uint32_t exp = mantissa_start_bit + 1U;
  return (exp << MANTISSA_BITS) + mantissa;
}

/// Bin index from size, rounding down so the bin is not larger than `size`.
[[nodiscard]] constexpr uint32_t UintToFloatRoundDown(uint32_t size) noexcept {
  if (size < MANTISSA_VALUE) {
    return size;
  }

  const uint32_t leading_zeros = std::countl_zero(size);
  const uint32_t highest_set_bit = 31U - leading_zeros;
  const uint32_t mantissa_start_bit = highest_set_bit - MANTISSA_BITS;
  const uint32_t mantissa = (size >> mantissa_start_bit) & MANTISSA_MASK;
  const uint32_t exp = mantissa_start_bit + 1U;
  return (exp << MANTISSA_BITS) | mantissa;
}

[[nodiscard]] constexpr uint32_t FloatToUint(uint32_t float_value) noexcept {
  const uint32_t exponent = float_value >> MANTISSA_BITS;
  const uint32_t mantissa = float_value & MANTISSA_MASK;
  if (exponent == 0) {
    return mantissa;
  }
  return (mantissa | MANTISSA_VALUE) << (exponent - 1U);
}

}  // namespace

void OffsetAllocator::Reset() noexcept {
  APERTURE_ASSERT(size_ != 0);
  APERTURE_ASSERT(max_allocs_ != 0);

  free_bytes_ = 0;
  used_bins_top_ = 0;
  free_offset_ = max_allocs_ - 1;
  used_count_ = 0;
  used_bins_.fill(0);
  bin_indices_.fill(Node::UNUSED);

  auto* storage = new (std::nothrow)
      std::byte[sizeof(Node) * max_allocs_ +
                (static_cast<size_t>(max_allocs_) + 7U) / 8U];
  APERTURE_ASSERT(storage != nullptr);
  storage_.reset(storage);
  std::memset(UsedBits(), 0, (static_cast<size_t>(max_allocs_) + 7U) / 8U);

  auto* free_nodes = new (std::nothrow) uint32_t[max_allocs_];
  APERTURE_ASSERT(free_nodes != nullptr);
  free_nodes_.reset(free_nodes);

  for (uint32_t i = 0; i < max_allocs_; ++i) {
    free_nodes_[i] = max_allocs_ - i - 1;
  }

  std::ignore = InsertNodeIntoBin(size_, 0);
}

auto OffsetAllocator::Allocate(uint32_t size) noexcept -> Allocation {
  APERTURE_ASSERT(size != 0);
  if (free_offset_ == 0) {
    return {};
  }

  const uint32_t min_bin_index = UintToFloatRoundUp(size);
  const uint32_t min_top_bin = min_bin_index >> TOP_BIN_SHIFT;
  const uint32_t min_leaf_bin = min_bin_index & LEAF_BIN_MASK;

  uint32_t top_bin = min_top_bin;
  uint32_t leaf_bin = Allocation::NO_SPACE;

  if ((used_bins_top_ & (1U << top_bin)) != 0) {
    leaf_bin = FindLowestSetBitAfter(used_bins_[top_bin], min_leaf_bin);
  }

  if (leaf_bin == Allocation::NO_SPACE) {
    top_bin = FindLowestSetBitAfter(used_bins_top_, min_top_bin + 1);
    if (top_bin == Allocation::NO_SPACE) {
      return {};
    }
    leaf_bin = std::countr_zero(used_bins_[top_bin]);
  }

  Node* nodes = Nodes();
  const uint32_t bin_index = (top_bin << TOP_BIN_SHIFT) | leaf_bin;
  const uint32_t node_index = bin_indices_[bin_index];
  Node& node = nodes[node_index];
  const uint32_t node_total_size = node.size;
  node.size = size;
  SetUsed(node_index, true);
  bin_indices_[bin_index] = node.bin_next;
  if (node.bin_next != Node::UNUSED) {
    nodes[node.bin_next].bin_prev = Node::UNUSED;
  }
  free_bytes_ -= node_total_size;

  if (bin_indices_[bin_index] == Node::UNUSED) {
    used_bins_[top_bin] =
        static_cast<uint8_t>(used_bins_[top_bin] & ~(1U << leaf_bin));
    if (used_bins_[top_bin] == 0) {
      used_bins_top_ &= ~(1U << top_bin);
    }
  }

  const uint32_t remainder = node_total_size - size;
  if (remainder > 0) {
    const uint32_t new_index = InsertNodeIntoBin(remainder, node.offset + size);
    if (node.neighbor_next != Node::UNUSED) {
      nodes[node.neighbor_next].neighbor_prev = new_index;
    }
    nodes[new_index].neighbor_prev = node_index;
    nodes[new_index].neighbor_next = node.neighbor_next;
    node.neighbor_next = new_index;
  }

  return {.offset = node.offset, .metadata = node_index};
}

void OffsetAllocator::Free(Allocation allocation) noexcept {
  if (!allocation) [[unlikely]] {
    return;
  }
  APERTURE_ASSERT(storage_ != nullptr);
  APERTURE_ASSERT(allocation.metadata != Allocation::NO_SPACE);

  Node* nodes = Nodes();
  const uint32_t node_index = allocation.metadata;
  Node& node = nodes[node_index];
  APERTURE_ASSERT(IsUsed(node_index));

  uint32_t offset = node.offset;
  uint32_t size = node.size;

  if (node.neighbor_prev != Node::UNUSED && !IsUsed(node.neighbor_prev)) {
    Node& prev = nodes[node.neighbor_prev];
    offset = prev.offset;
    size += prev.size;
    RemoveNodeFromBin(node.neighbor_prev);
    APERTURE_ASSERT(prev.neighbor_next == node_index);
    node.neighbor_prev = prev.neighbor_prev;
  }

  if (node.neighbor_next != Node::UNUSED && !IsUsed(node.neighbor_next)) {
    Node& next = nodes[node.neighbor_next];
    size += next.size;
    RemoveNodeFromBin(node.neighbor_next);
    APERTURE_ASSERT(next.neighbor_prev == node_index);
    node.neighbor_next = next.neighbor_next;
  }

  const uint32_t neighbor_next = node.neighbor_next;
  const uint32_t neighbor_prev = node.neighbor_prev;
  SetUsed(node_index, false);
  free_nodes_[++free_offset_] = node_index;

  const uint32_t combined = InsertNodeIntoBin(size, offset);
  if (neighbor_next != Node::UNUSED) {
    nodes[combined].neighbor_next = neighbor_next;
    nodes[neighbor_next].neighbor_prev = combined;
  }
  if (neighbor_prev != Node::UNUSED) {
    nodes[combined].neighbor_prev = neighbor_prev;
    nodes[neighbor_prev].neighbor_next = combined;
  }
}

auto OffsetAllocator::Report() const noexcept -> StorageReport {
  StorageReport report{};
  if (free_offset_ == 0) {
    return report;
  }
  report.free_bytes = free_bytes_;
  if (used_bins_top_ != 0) {
    const uint32_t top_bin = 31U - std::countl_zero(used_bins_top_);
    const uint32_t leaf_bin =
        31U - std::countl_zero(static_cast<uint32_t>(used_bins_[top_bin]));
    report.largest_free_region =
        FloatToUint((top_bin << TOP_BIN_SHIFT) | leaf_bin);
  }
  return report;
}

auto OffsetAllocator::ReportFull() const noexcept -> StorageReportFull {
  StorageReportFull report{};
  const Node* nodes = Nodes();
  for (uint32_t i = 0; i < LEAF_BIN_COUNT; ++i) {
    uint32_t count = 0;
    uint32_t node_index = bin_indices_[i];
    while (node_index != Node::UNUSED) {
      node_index = nodes[node_index].bin_next;
      ++count;
    }
    report.free_regions[i] = {
        .size = FloatToUint(i),
        .count = count,
    };
  }
  return report;
}

uint32_t OffsetAllocator::InsertNodeIntoBin(uint32_t size,
                                            uint32_t data_offset) noexcept {
  const uint32_t bin_index = UintToFloatRoundDown(size);
  const uint32_t top_bin = bin_index >> TOP_BIN_SHIFT;
  const uint32_t leaf_bin = bin_index & LEAF_BIN_MASK;

  if (bin_indices_[bin_index] == Node::UNUSED) {
    used_bins_[top_bin] =
        static_cast<uint8_t>(used_bins_[top_bin] | (1u << leaf_bin));
    used_bins_top_ |= 1u << top_bin;
  }

  Node* nodes = Nodes();
  const uint32_t top_node_index = bin_indices_[bin_index];
  const uint32_t node_index = free_nodes_[free_offset_--];
  nodes[node_index] = Node{
      .offset = data_offset,
      .size = size,
      .bin_next = top_node_index,
  };
  SetUsed(node_index, false);
  if (top_node_index != Node::UNUSED) {
    nodes[top_node_index].bin_prev = node_index;
  }
  bin_indices_[bin_index] = node_index;
  free_bytes_ += size;
  return node_index;
}

void OffsetAllocator::RemoveNodeFromBin(uint32_t node_index) noexcept {
  Node* nodes = Nodes();
  Node& node = nodes[node_index];
  if (node.bin_prev != Node::UNUSED) {
    nodes[node.bin_prev].bin_next = node.bin_next;
    if (node.bin_next != Node::UNUSED) {
      nodes[node.bin_next].bin_prev = node.bin_prev;
    }
  } else {
    const uint32_t bin_index = UintToFloatRoundDown(node.size);
    const uint32_t top_bin = bin_index >> TOP_BIN_SHIFT;
    const uint32_t leaf_bin = bin_index & LEAF_BIN_MASK;
    bin_indices_[bin_index] = node.bin_next;
    if (node.bin_next != Node::UNUSED) {
      nodes[node.bin_next].bin_prev = Node::UNUSED;
    }
    if (bin_indices_[bin_index] == Node::UNUSED) {
      used_bins_[top_bin] =
          static_cast<uint8_t>(used_bins_[top_bin] & ~(1U << leaf_bin));
      if (used_bins_[top_bin] == 0) {
        used_bins_top_ &= ~(1U << top_bin);
      }
    }
  }

  free_nodes_[++free_offset_] = node_index;
  free_bytes_ -= node.size;
}

void OffsetAllocator::MoveFrom(OffsetAllocator* other) noexcept {
  APERTURE_ASSERT(other != nullptr);
  storage_ = std::move(other->storage_);
  free_nodes_ = std::move(other->free_nodes_);
  bin_indices_ = other->bin_indices_;
  size_ = other->size_;
  max_allocs_ = other->max_allocs_;
  free_bytes_ = other->free_bytes_;
  used_bins_top_ = other->used_bins_top_;
  free_offset_ = other->free_offset_;
  used_bins_ = other->used_bins_;
  used_count_ = other->used_count_;

  other->bin_indices_.fill(Node::UNUSED);
  other->size_ = 0;
  other->max_allocs_ = 0;
  other->free_bytes_ = 0;
  other->used_bins_top_ = 0;
  other->free_offset_ = 0;
  other->used_bins_.fill(0);
  other->used_count_ = 0;
}

}  // namespace aperture
