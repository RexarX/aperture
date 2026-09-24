#include <pch.hpp>

#include <aperture/queue.hpp>

#include <aperture/assert.hpp>
#include <aperture/device.hpp>
#include <aperture/result.hpp>

#ifdef APERTURE_HAS_VULKAN
#include <aperture/vulkan/queue.hpp>
#endif

#include <expected>

namespace aperture {

Queue GraphicsQueue(Device device) noexcept {
  APERTURE_ASSERT(device.ptr != nullptr);

  switch (BackendOf(device)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      auto* vk_device = static_cast<vk::Device*>(device.ptr);
      return {.ptr = vk::GraphicsQueue(vk_device)};
    }
#endif
    default:
      APERTURE_ASSERT(false, "Unsupported backend: {}!",
                      ToString(BackendOf(device)));
      return {};
  }
}

auto ComputeQueue(Device device) noexcept -> Result<Queue> {
  APERTURE_ASSERT(device.ptr != nullptr);

  switch (BackendOf(device)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      auto* vk_device = static_cast<vk::Device*>(device.ptr);
      auto queue = vk::ComputeQueue(vk_device);
      if (!queue) [[unlikely]] {
        return std::unexpected(queue.error());
      }
      return Queue{.ptr = *queue};
    }
#endif
    default:
      APERTURE_ASSERT(false, "Unsupported backend: {} ({})",
                      ToString(BackendOf(device)),
                      ToString(Error::Unsupported));
      return std::unexpected(Error::Unsupported);
  }
}

auto CopyQueue(Device device) noexcept -> Result<Queue> {
  APERTURE_ASSERT(device.ptr != nullptr);

  switch (BackendOf(device)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      auto* vk_device = static_cast<vk::Device*>(device.ptr);
      auto queue = vk::CopyQueue(vk_device);
      if (!queue) [[unlikely]] {
        return std::unexpected(queue.error());
      }
      return Queue{.ptr = *queue};
    }
#endif
    default:
      APERTURE_ASSERT(false, "Unsupported backend: {} ({})",
                      ToString(BackendOf(device)),
                      ToString(Error::Unsupported));
      return std::unexpected(Error::Unsupported);
  }
}

}  // namespace aperture
