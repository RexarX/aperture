#ifndef APERTURE_MEMORY_OFFSET_ALLOCATOR_H
#define APERTURE_MEMORY_OFFSET_ALLOCATOR_H

#include <aperture/platform.h>

#include <stdbool.h>
#include <stdint.h>

APERTURE_C_BEGIN

#define APERTURE_OFFSET_ALLOCATOR_DEFAULT_MAX_ALLOCS (128U * 1024U)
#define APERTURE_OFFSET_ALLOCATION_NO_SPACE 0xFFFFFFFFU

/// @brief One contiguous range inside managed storage.
typedef struct ApertureOffsetAllocation {
  uint32_t offset;
  uint32_t metadata;
} ApertureOffsetAllocation;

/// @brief Coarse free-space query.
typedef struct ApertureOffsetStorageReport {
  uint32_t free_bytes;
  uint32_t largest_free_region;
} ApertureOffsetStorageReport;

/// @brief True if this is a live allocation.
/// @param allocation Allocation to test
/// @return `true` if `offset` is not `APERTURE_OFFSET_ALLOCATION_NO_SPACE`
static inline bool aperture_offset_allocation_ok(
    ApertureOffsetAllocation allocation) {
  return allocation.offset != APERTURE_OFFSET_ALLOCATION_NO_SPACE;
}

/// @brief TLSF-style offset allocator. Heap-allocated wrapper around
/// `aperture::OffsetAllocator`. Invalid is `NULL`.
typedef struct ApertureOffsetAllocatorImpl* ApertureOffsetAllocator;

/// @brief Takes ownership of a `[0, size)` range to suballocate.
/// @param size Byte count of the managed storage. Must be non-zero
/// @param max_allocs Maximum live allocations. Must be non-zero
/// @return Allocator, or `NULL` on allocation failure
/// @warning Asserts if `size` or `max_allocs` is 0.
APERTURE_C_API ApertureOffsetAllocator aperture_offset_allocator_create(
    uint32_t size, uint32_t max_allocs) APERTURE_C_NOEXCEPT;

/// @brief Destroys an offset allocator. Null is a no-op.
/// @param allocator Allocator to destroy
APERTURE_C_API void aperture_offset_allocator_destroy(
    ApertureOffsetAllocator allocator) APERTURE_C_NOEXCEPT;

/// @brief Drops every allocation and restores a single free region.
/// @param allocator Allocator to reset
/// @warning Asserts if `allocator` is null.
APERTURE_C_API void aperture_offset_allocator_reset(
    ApertureOffsetAllocator allocator) APERTURE_C_NOEXCEPT;

/// @brief Allocates `size` contiguous bytes.
/// @param allocator Allocator that owns the range
/// @param size Byte count. Must be non-zero
/// @return Live allocation, or `offset == APERTURE_OFFSET_ALLOCATION_NO_SPACE`
/// @warning Asserts if `allocator` is null or `size` is 0.
APERTURE_C_API ApertureOffsetAllocation aperture_offset_allocator_allocate(
    ApertureOffsetAllocator allocator, uint32_t size) APERTURE_C_NOEXCEPT;

/// @brief Returns `allocation` to the pool and coalesces neighbors.
/// @param allocator Allocator that owns the range
/// @param allocation Live allocation from `aperture_offset_allocator_allocate`
/// @warning Asserts if `allocator` is null or `allocation` is not live.
APERTURE_C_API void aperture_offset_allocator_free(
    ApertureOffsetAllocator allocator,
    ApertureOffsetAllocation allocation) APERTURE_C_NOEXCEPT;

/// @brief Byte count of `allocation`.
/// @param allocator Allocator that owns the range
/// @param allocation Live allocation to query
/// @return Size in bytes
/// @warning Asserts if `allocator` is null.
APERTURE_C_API uint32_t aperture_offset_allocator_allocation_size(
    ApertureOffsetAllocator allocator,
    ApertureOffsetAllocation allocation) APERTURE_C_NOEXCEPT;

/// @brief True if every byte is free.
/// @param allocator Allocator to query
/// @return `true` if no live allocations remain
/// @warning Asserts if `allocator` is null.
APERTURE_C_API bool aperture_offset_allocator_empty(
    ApertureOffsetAllocator allocator) APERTURE_C_NOEXCEPT;

/// @brief Gets free bytes and a lower bound on the largest free region.
/// @param allocator Allocator to query
/// @return Coarse free-space report
/// @warning Asserts if `allocator` is null.
APERTURE_C_API ApertureOffsetStorageReport aperture_offset_allocator_report(
    ApertureOffsetAllocator allocator) APERTURE_C_NOEXCEPT;

/// @brief Managed storage size in bytes.
/// @param allocator Allocator to query
/// @return Size passed to `aperture_offset_allocator_create`
/// @warning Asserts if `allocator` is null.
APERTURE_C_API uint32_t aperture_offset_allocator_size(
    ApertureOffsetAllocator allocator) APERTURE_C_NOEXCEPT;

/// @brief Currently free bytes (may be fragmented).
/// @param allocator Allocator to query
/// @return Free byte count
/// @warning Asserts if `allocator` is null.
APERTURE_C_API uint32_t aperture_offset_allocator_free_bytes(
    ApertureOffsetAllocator allocator) APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
