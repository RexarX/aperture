#include <pch.hpp>

#include <aperture/vulkan/memory/malloc.hpp>

#include <aperture/assert.hpp>
#include <aperture/commands.hpp>
#include <aperture/queue.hpp>
#include <aperture/result.hpp>
#include <aperture/types.hpp>
#include <aperture/utils/bit.hpp>

#include "internal.hpp"
#include "storage.hpp"

#include <cstddef>
#include <expected>

namespace aperture::vk {

namespace {

[[nodiscard]] auto MallocImpl(Device* device, size_t bytes, size_t align,
                              Memory memory, QueueUsage usage,
                              bool dedicated) noexcept
    -> Result<DualPtr<std::byte>> {
  APERTURE_ASSERT(device != nullptr);
  APERTURE_ASSERT(device->memory != nullptr);

  const bool log = device->log_failed_results;

  if (bytes == 0 || !utils::IsPowerOfTwo(align)) [[unlikely]] {
    LogFailed(log, "Malloc size/align is invalid ({})!",
              ToString(Error::Invalid));
    return std::unexpected(Error::Invalid);
  }

  if (memory != Memory::Default && memory != Memory::Readback) [[unlikely]] {
    LogFailed(log, "Malloc memory class is invalid ({})!",
              ToString(Error::Invalid));
    return std::unexpected(Error::Invalid);
  }
  if (auto valid = ValidateUsage(*device, usage, log); !valid) [[unlikely]] {
    return std::unexpected(valid.error());
  }

  auto rec = AllocateFromHeaps(device, KindOf(memory), usage, bytes, align,
                               dedicated, log);
  if (!rec) [[unlikely]] {
    return std::unexpected(rec.error());
  }
  return DualPtr<std::byte>{
      .host = static_cast<std::byte*>(rec->host),
      .device = {.addr = rec->device_addr},
  };
}

[[nodiscard]] auto MallocGpuImpl(Device* device, size_t bytes, size_t align,
                                 QueueUsage usage, bool dedicated) noexcept
    -> Result<GpuPtr<std::byte>> {
  APERTURE_ASSERT(device != nullptr);
  APERTURE_ASSERT(device->memory != nullptr);

  const bool log = device->log_failed_results;

  if (bytes == 0 || !utils::IsPowerOfTwo(align)) [[unlikely]] {
    LogFailed(log, "MallocGpu size/align is invalid ({})!",
              ToString(Error::Invalid));
    return std::unexpected(Error::Invalid);
  }

  if (auto valid = ValidateUsage(*device, usage, log); !valid) [[unlikely]] {
    return std::unexpected(valid.error());
  }

  auto rec = AllocateFromHeaps(device, HeapKind::Gpu, usage, bytes, align,
                               dedicated, log);
  if (!rec) [[unlikely]] {
    return std::unexpected(rec.error());
  }
  return GpuPtr<std::byte>{.addr = rec->device_addr};
}

}  // namespace

auto Malloc(Device* device, size_t bytes, size_t align, Memory memory,
            QueueUsage usage, MallocFlags flags) noexcept
    -> Result<DualPtr<std::byte>> {
  return MallocImpl(device, bytes, align, memory, usage,
                    HasAll(flags, MallocFlags::Dedicated));
}

auto MallocGpu(Device* device, size_t bytes, size_t align, QueueUsage usage,
               MallocFlags flags) noexcept -> Result<GpuPtr<std::byte>> {
  return MallocGpuImpl(device, bytes, align, usage,
                       HasAll(flags, MallocFlags::Dedicated));
}

}  // namespace aperture::vk
