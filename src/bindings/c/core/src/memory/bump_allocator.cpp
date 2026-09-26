#include <aperture/memory/bump_allocator.h>

#include <aperture/assert.hpp>
#include <aperture/memory/bump_allocator.hpp>

#include <cstdbool>
#include <cstdint>

extern "C" {

ApertureBumpAllocator aperture_bump_allocator_create(uint64_t size) noexcept {
  APERTURE_ASSERT(size != 0);
  auto* allocator = new aperture::BumpAllocator{size};
  return reinterpret_cast<ApertureBumpAllocator>(allocator);
}

void aperture_bump_allocator_destroy(ApertureBumpAllocator allocator) noexcept {
  auto* alloc = reinterpret_cast<aperture::BumpAllocator*>(allocator);
  delete alloc;
}

void aperture_bump_allocator_reset(ApertureBumpAllocator allocator) noexcept {
  APERTURE_ASSERT(allocator != nullptr);
  auto* alloc = reinterpret_cast<aperture::BumpAllocator*>(allocator);
  alloc->Reset();
}

ApertureBumpAllocation aperture_bump_allocator_allocate(
    ApertureBumpAllocator allocator, uint64_t size, uint64_t align) noexcept {
  APERTURE_ASSERT(allocator != nullptr);

  auto* alloc = reinterpret_cast<aperture::BumpAllocator*>(allocator);
  const auto allocation = alloc->Allocate(size, align);
  return {
      .offset = allocation.offset,
      .size = allocation.size,
  };
}

bool aperture_bump_allocator_empty(ApertureBumpAllocator allocator) noexcept {
  APERTURE_ASSERT(allocator != nullptr);
  auto* alloc = reinterpret_cast<aperture::BumpAllocator*>(allocator);
  return alloc->Empty();
}

uint64_t aperture_bump_allocator_size(
    ApertureBumpAllocator allocator) noexcept {
  APERTURE_ASSERT(allocator != nullptr);
  auto* alloc = reinterpret_cast<aperture::BumpAllocator*>(allocator);
  return alloc->Size();
}

uint64_t aperture_bump_allocator_used(
    ApertureBumpAllocator allocator) noexcept {
  APERTURE_ASSERT(allocator != nullptr);
  auto* alloc = reinterpret_cast<aperture::BumpAllocator*>(allocator);
  return alloc->Used();
}

uint64_t aperture_bump_allocator_free_bytes(
    ApertureBumpAllocator allocator) noexcept {
  APERTURE_ASSERT(allocator != nullptr);
  auto* alloc = reinterpret_cast<aperture::BumpAllocator*>(allocator);
  return alloc->FreeBytes();
}
}
