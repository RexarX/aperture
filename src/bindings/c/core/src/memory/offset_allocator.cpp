#include <aperture/memory/offset_allocator.h>

#include <aperture/assert.hpp>
#include <aperture/memory/offset_allocator.hpp>

#include <stdbool.h>
#include <stdint.h>

extern "C" {

ApertureOffsetAllocator aperture_offset_allocator_create(
    uint32_t size, uint32_t max_allocs) noexcept {
  APERTURE_ASSERT(size != 0);
  APERTURE_ASSERT(max_allocs != 0);
  auto* allocator = new aperture::OffsetAllocator{size, max_allocs};
  return reinterpret_cast<ApertureOffsetAllocator>(allocator);
}

void aperture_offset_allocator_destroy(
    ApertureOffsetAllocator allocator) noexcept {
  auto* alloc = reinterpret_cast<aperture::OffsetAllocator*>(allocator);
  delete alloc;
}

void aperture_offset_allocator_reset(
    ApertureOffsetAllocator allocator) noexcept {
  APERTURE_ASSERT(allocator != nullptr);
  auto* alloc = reinterpret_cast<aperture::OffsetAllocator*>(allocator);
  alloc->Reset();
}

ApertureOffsetAllocation aperture_offset_allocator_allocate(
    ApertureOffsetAllocator allocator, uint32_t size) noexcept {
  APERTURE_ASSERT(allocator != nullptr);
  auto* alloc = reinterpret_cast<aperture::OffsetAllocator*>(allocator);
  const auto allocation = alloc->Allocate(size);
  return {
      .offset = allocation.offset,
      .metadata = allocation.metadata,
  };
}

void aperture_offset_allocator_free(
    ApertureOffsetAllocator allocator,
    ApertureOffsetAllocation allocation) noexcept {
  APERTURE_ASSERT(allocator != nullptr);
  auto* alloc = reinterpret_cast<aperture::OffsetAllocator*>(allocator);
  alloc->Free({.offset = allocation.offset, .metadata = allocation.metadata});
}

uint32_t aperture_offset_allocator_allocation_size(
    ApertureOffsetAllocator allocator,
    ApertureOffsetAllocation allocation) noexcept {
  APERTURE_ASSERT(allocator != nullptr);
  auto* alloc = reinterpret_cast<aperture::OffsetAllocator*>(allocator);
  return alloc->AllocationSize(
      {.offset = allocation.offset, .metadata = allocation.metadata});
}

bool aperture_offset_allocator_empty(
    ApertureOffsetAllocator allocator) noexcept {
  APERTURE_ASSERT(allocator != nullptr);
  auto* alloc = reinterpret_cast<aperture::OffsetAllocator*>(allocator);
  return alloc->Empty();
}

ApertureOffsetStorageReport aperture_offset_allocator_report(
    ApertureOffsetAllocator allocator) noexcept {
  APERTURE_ASSERT(allocator != nullptr);
  auto* alloc = reinterpret_cast<aperture::OffsetAllocator*>(allocator);
  const auto report = alloc->Report();
  return {
      .free_bytes = report.free_bytes,
      .largest_free_region = report.largest_free_region,
  };
}

uint32_t aperture_offset_allocator_size(
    ApertureOffsetAllocator allocator) noexcept {
  APERTURE_ASSERT(allocator != nullptr);
  auto* alloc = reinterpret_cast<aperture::OffsetAllocator*>(allocator);
  return alloc->Size();
}

uint32_t aperture_offset_allocator_free_bytes(
    ApertureOffsetAllocator allocator) noexcept {
  APERTURE_ASSERT(allocator != nullptr);
  auto* alloc = reinterpret_cast<aperture::OffsetAllocator*>(allocator);
  return alloc->FreeBytes();
}
}
