#include <pch.hpp>

#include <aperture/memory/malloc.hpp>

#include <aperture/assert.hpp>
#include <aperture/commands.hpp>
#include <aperture/device.hpp>
#include <aperture/log.hpp>
#include <aperture/queue.hpp>
#include <aperture/result.hpp>
#include <aperture/types.hpp>

#ifdef APERTURE_HAS_VULKAN
#include <aperture/vulkan/memory/malloc.hpp>
#endif

#include <cstddef>
#include <expected>

namespace aperture {

auto Malloc(Device device, size_t bytes, size_t align, Memory memory,
            QueueUsage usage) noexcept -> Result<DualPtr<std::byte>> {
  APERTURE_ASSERT(device.ptr != nullptr);

  switch (BackendOf(device)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      auto* vk_device = static_cast<vk::Device*>(device.ptr);
      return vk::Malloc(vk_device, bytes, align, memory, usage);
    }
#endif
    default:
      log::Error("Unsupported backend: {} ({})!", ToString(BackendOf(device)),
                 ToString(Error::Unsupported));
      return std::unexpected(Error::Unsupported);
  }
}

auto MallocDedicated(Device device, size_t bytes, size_t align, Memory memory,
                     QueueUsage usage) noexcept -> Result<DualPtr<std::byte>> {
  APERTURE_ASSERT(device.ptr != nullptr);

  switch (BackendOf(device)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      auto* vk_device = static_cast<vk::Device*>(device.ptr);
      return vk::MallocDedicated(vk_device, bytes, align, memory, usage);
    }
#endif
    default:
      log::Error("Unsupported backend: {} ({})!", ToString(BackendOf(device)),
                 ToString(Error::Unsupported));
      return std::unexpected(Error::Unsupported);
  }
}

auto MallocGpu(Device device, size_t bytes, size_t align,
               QueueUsage usage) noexcept -> Result<GpuPtr<std::byte>> {
  APERTURE_ASSERT(device.ptr != nullptr);

  switch (BackendOf(device)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      auto* vk_device = static_cast<vk::Device*>(device.ptr);
      return vk::MallocGpu(vk_device, bytes, align, usage);
    }
#endif
    default:
      log::Error("Unsupported backend: {} ({})!", ToString(BackendOf(device)),
                 ToString(Error::Unsupported));
      return std::unexpected(Error::Unsupported);
  }
}

auto MallocGpuDedicated(Device device, size_t bytes, size_t align,
                        QueueUsage usage) noexcept
    -> Result<GpuPtr<std::byte>> {
  APERTURE_ASSERT(device.ptr != nullptr);

  switch (BackendOf(device)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      auto* vk_device = static_cast<vk::Device*>(device.ptr);
      return vk::MallocGpuDedicated(vk_device, bytes, align, usage);
    }
#endif
    default:
      log::Error("Unsupported backend: {} ({})!", ToString(BackendOf(device)),
                 ToString(Error::Unsupported));
      return std::unexpected(Error::Unsupported);
  }
}

}  // namespace aperture
