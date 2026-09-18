#include <pch.hpp>

#include <aperture/vulkan/queue.hpp>

#include <aperture/assert.hpp>
#include <aperture/result.hpp>
#include <aperture/vulkan/device.hpp>
#include "internal.hpp"

#include <expected>

namespace aperture::vk {

Queue* GraphicsQueue(Device* device) noexcept {
  APERTURE_ASSERT(device != nullptr);
  return &device->graphics;
}

auto ComputeQueue(Device* device) noexcept -> Result<Queue*> {
  APERTURE_ASSERT(device != nullptr);
  if (!device->has_compute) [[unlikely]] {
    LogFailed(device->log_failed_results,
              "Device has no async compute queue ({})!",
              ToString(Error::Unsupported));
    return std::unexpected(Error::Unsupported);
  }
  return &device->compute;
}

auto CopyQueue(Device* device) noexcept -> Result<Queue*> {
  APERTURE_ASSERT(device != nullptr);
  if (!device->has_copy) [[unlikely]] {
    LogFailed(device->log_failed_results,
              "Device has no async copy queue ({})!",
              ToString(Error::Unsupported));
    return std::unexpected(Error::Unsupported);
  }
  return &device->copy;
}

Queue& GetNative(aperture::Queue queue) noexcept {
  APERTURE_ASSERT(queue.ptr != nullptr);
  auto& impl = *static_cast<Queue*>(queue.ptr);
  APERTURE_ASSERT(impl.device != nullptr);
  APERTURE_ASSERT(impl.device->backend == Backend::Vulkan);
  return impl;
}

}  // namespace aperture::vk
