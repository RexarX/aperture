#include <pch.hpp>

#include <aperture/command/pool.hpp>

#include <aperture/assert.hpp>
#include <aperture/log.hpp>
#include <aperture/queue.hpp>
#include <aperture/result.hpp>
#include <aperture/types.hpp>

#ifdef APERTURE_HAS_VULKAN
#include <aperture/vulkan/command/pool.hpp>
#include <aperture/vulkan/device.hpp>
#include <aperture/vulkan/queue.hpp>
#endif

#include <expected>

namespace aperture {

namespace {

[[nodiscard]] constexpr Backend BackendOfPtr(const void* ptr) noexcept {
  APERTURE_ASSERT(ptr != nullptr);
  return *static_cast<const Backend*>(ptr);
}

}  // namespace

auto CreateCommandPool(Queue queue, const CommandPoolDesc& desc) noexcept
    -> Result<CommandPool> {
  APERTURE_ASSERT(queue.ptr != nullptr);

#ifdef APERTURE_HAS_VULKAN
  auto* vk_queue = static_cast<vk::Queue*>(queue.ptr);
  APERTURE_ASSERT(vk_queue->device != nullptr);
  switch (vk_queue->device->backend) {
    using enum Backend;
    case Vulkan: {
      auto created = vk::CreateCommandPool(vk_queue, desc);
      if (!created) [[unlikely]] {
        return std::unexpected(created.error());
      }
      return CommandPool{.ptr = *created};
    }
    default:
      break;
  }
  const Backend backend = vk_queue->device->backend;
#else
  const Backend backend = Backend::Vulkan;
#endif
  log::Error("Unsupported backend: {} ({})!", ToString(backend),
             ToString(Error::Unsupported));
  return std::unexpected(Error::Unsupported);
}

void Reset(CommandPool pool) noexcept {
  if (!pool) {
    return;
  }

  switch (BackendOfPtr(pool.ptr)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan:
      vk::Reset(static_cast<vk::CommandPool*>(pool.ptr));
      return;
#endif
    default:
      APERTURE_ASSERT(false, "Unsupported backend: {}!",
                      ToString(BackendOfPtr(pool.ptr)));
  }
}

void Destroy(CommandPool pool) noexcept {
  if (!pool) {
    return;
  }

  switch (BackendOfPtr(pool.ptr)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan:
      vk::Destroy(static_cast<vk::CommandPool*>(pool.ptr));
      return;
#endif
    default:
      APERTURE_ASSERT(false, "Unsupported backend: {}!",
                      ToString(BackendOfPtr(pool.ptr)));
  }
}

}  // namespace aperture
