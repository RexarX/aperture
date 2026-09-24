#include <pch.hpp>

#include <aperture/memory/bump_allocator.hpp>

#include <aperture/assert.hpp>
#include <aperture/utils/bit.hpp>

#include <atomic>
#include <cstdint>

namespace aperture {

static_assert(std::atomic<uint64_t>::is_always_lock_free);

auto BumpAllocator::Allocate(uint64_t size,
                             [[maybe_unused]] uint64_t align) noexcept
    -> Allocation {
  APERTURE_ASSERT(size != 0);
  APERTURE_ASSERT(utils::IsPowerOfTwo(align));
  APERTURE_ASSERT(align <= DEFAULT_ALIGN);
  APERTURE_ASSERT(size_ != 0);

  // Fixed quantum keeps the cursor DEFAULT_ALIGN-aligned from offset 0, so any
  // power-of-two align <= DEFAULT_ALIGN is satisfied without a CAS.
  const uint64_t reservation = utils::AlignUp(size, DEFAULT_ALIGN);
  const uint64_t offset =
      offset_.fetch_add(reservation, std::memory_order_relaxed);
  if (reservation > size_ || offset > size_ - reservation) [[unlikely]] {
    return {};
  }
  return {.offset = offset, .size = size};
}

void BumpAllocator::MoveFrom(BumpAllocator* other) noexcept {
  APERTURE_ASSERT(other != nullptr);

  size_ = other->size_;
  offset_.store(other->offset_.load(std::memory_order_relaxed),
                std::memory_order_relaxed);
  other->size_ = 0;
  other->offset_.store(0, std::memory_order_relaxed);
}

}  // namespace aperture
