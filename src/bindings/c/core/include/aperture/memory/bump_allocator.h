#ifndef APERTURE_MEMORY_BUMP_ALLOCATOR_H
#define APERTURE_MEMORY_BUMP_ALLOCATOR_H

#include <aperture/platform.h>

#include <stdbool.h>
#include <stdint.h>

APERTURE_C_BEGIN

/// @brief Default bump reservation alignment in bytes.
#define APERTURE_BUMP_ALLOCATOR_DEFAULT_ALIGN UINT64_C(16)

/// @brief Sentinel `offset` when a bump allocate fails.
#define APERTURE_BUMP_ALLOCATION_NO_SPACE (~UINT64_C(0))

/// @brief One contiguous range inside managed storage.
typedef struct ApertureBumpAllocation {
  uint64_t offset;
  uint64_t size;
} ApertureBumpAllocation;

/// @brief True if this is a live allocation.
/// @param allocation Allocation to test
/// @return `true` if `offset` is not `APERTURE_BUMP_ALLOCATION_NO_SPACE`
static inline bool aperture_bump_allocation_ok(
    ApertureBumpAllocation allocation) {
  return allocation.offset != APERTURE_BUMP_ALLOCATION_NO_SPACE;
}

/// @brief Wait-free bump allocator over a `[0, size)` range. Heap-allocated
/// wrapper around `aperture::BumpAllocator`. Invalid is `NULL`.
typedef struct ApertureBumpAllocatorImpl* ApertureBumpAllocator;

/// @brief Takes ownership of a `[0, size)` range to suballocate.
/// @param size Byte count of the managed storage. Must be non-zero
/// @return Allocator, or `NULL` on allocation failure
/// @warning Asserts if `size` is 0.
APERTURE_C_API ApertureBumpAllocator
aperture_bump_allocator_create(uint64_t size) APERTURE_C_NOEXCEPT;

/// @brief Destroys a bump allocator. Null is a no-op.
/// @param allocator Allocator to destroy
APERTURE_C_API void aperture_bump_allocator_destroy(
    ApertureBumpAllocator allocator) APERTURE_C_NOEXCEPT;

/// @brief Rewinds the cursor to empty. Live allocations become invalid.
/// @param allocator Allocator to reset
/// @warning Must not run concurrently with allocate. Asserts if `allocator`
/// is null.
APERTURE_C_API void aperture_bump_allocator_reset(
    ApertureBumpAllocator allocator) APERTURE_C_NOEXCEPT;

/// @brief Allocates `size` contiguous bytes.
/// @param allocator Allocator that owns the range
/// @param size Byte count. Must be non-zero
/// @param align Power of two, at most `APERTURE_BUMP_ALLOCATOR_DEFAULT_ALIGN`
/// @return Live allocation, or `offset == APERTURE_BUMP_ALLOCATION_NO_SPACE`
/// @warning Asserts if `allocator` is null, `size` is 0, or `align` is invalid.
APERTURE_C_API ApertureBumpAllocation
aperture_bump_allocator_allocate(ApertureBumpAllocator allocator, uint64_t size,
                                 uint64_t align) APERTURE_C_NOEXCEPT;

/// @brief True if the cursor is at zero.
/// @param allocator Allocator to query
/// @return `true` if no bytes have been consumed
/// @warning Asserts if `allocator` is null.
APERTURE_C_API bool aperture_bump_allocator_empty(
    ApertureBumpAllocator allocator) APERTURE_C_NOEXCEPT;

/// @brief Managed storage size in bytes.
/// @param allocator Allocator to query
/// @return Size passed to `aperture_bump_allocator_create`
/// @warning Asserts if `allocator` is null.
APERTURE_C_API uint64_t aperture_bump_allocator_size(
    ApertureBumpAllocator allocator) APERTURE_C_NOEXCEPT;

/// @brief Bytes consumed by the cursor, including alignment padding.
/// @param allocator Allocator to query
/// @return Used byte count
/// @warning Asserts if `allocator` is null.
APERTURE_C_API uint64_t aperture_bump_allocator_used(
    ApertureBumpAllocator allocator) APERTURE_C_NOEXCEPT;

/// @brief Remaining bytes after the cursor.
/// @param allocator Allocator to query
/// @return Free byte count
/// @warning Asserts if `allocator` is null.
APERTURE_C_API uint64_t aperture_bump_allocator_free_bytes(
    ApertureBumpAllocator allocator) APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
