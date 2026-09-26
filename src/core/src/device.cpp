#include <pch.hpp>

#include <aperture/device.hpp>

#include <aperture/assert.hpp>
#include <aperture/capability.hpp>
#include <aperture/instance.hpp>
#include <aperture/log.hpp>
#include <aperture/result.hpp>
#include <aperture/types.hpp>

#ifdef APERTURE_HAS_VULKAN
#include <aperture/vulkan/device.hpp>
#endif

#include <expected>

namespace aperture {

Backend BackendOf(Device device) noexcept {
  APERTURE_ASSERT(device.ptr != nullptr);
  return *static_cast<const Backend*>(device.ptr);
}

auto CreateDevice(Instance instance, const DeviceDesc& desc) noexcept
    -> Result<Device> {
  APERTURE_ASSERT(instance.ptr != nullptr);

  switch (BackendOf(instance)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      auto* vk_instance = static_cast<vk::Instance*>(instance.ptr);
      auto created = vk::CreateDevice(vk_instance, desc);
      if (!created) [[unlikely]] {
        return std::unexpected(created.error());
      }
      return Device{.ptr = *created};
    }
#endif
    default:
      log::Error("Unsupported backend: {} ({})!", ToString(BackendOf(instance)),
                 ToString(Error::Unsupported));
      return std::unexpected(Error::Unsupported);
  }
}

void Destroy(Device device) noexcept {
  if (!device) [[unlikely]] {
    return;
  }

  switch (BackendOf(device)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      auto* vk_device = static_cast<vk::Device*>(device.ptr);
      vk::Destroy(vk_device);
      return;
    }
#endif
    default:
      APERTURE_ASSERT(false, "Unsupported backend: {}!",
                      ToString(BackendOf(device)));
  }
}

bool Has(Device device, Capability caps) noexcept {
  APERTURE_ASSERT(device.ptr != nullptr);

  switch (BackendOf(device)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      const auto& vk_device = *static_cast<const vk::Device*>(device.ptr);
      return vk::Has(vk_device, caps);
    }
#endif
    default:
      APERTURE_ASSERT(false, "Unsupported backend: {}!",
                      ToString(BackendOf(device)));
      return false;
  }
}

auto WaitIdle(Device device) noexcept -> Result<void> {
  APERTURE_ASSERT(device.ptr != nullptr);

  switch (BackendOf(device)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan:
      return vk::WaitIdle(static_cast<vk::Device*>(device.ptr));
#endif
    default:
      log::Error("Unsupported backend: {} ({})!", ToString(BackendOf(device)),
                 ToString(Error::Unsupported));
      return std::unexpected(Error::Unsupported);
  }
}

const DeviceInfo& Info(Device device) noexcept {
  APERTURE_ASSERT(device.ptr != nullptr);

  switch (BackendOf(device)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      const auto& vk_device = *static_cast<const vk::Device*>(device.ptr);
      return vk::Info(vk_device);
    }
#endif
    default: {
      APERTURE_ASSERT(false, "Unsupported backend: {}!",
                      ToString(BackendOf(device)));
      static const DeviceInfo empty{};
      return empty;
    }
  }
}

}  // namespace aperture
