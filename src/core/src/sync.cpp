#include <pch.hpp>

#include <aperture/sync.hpp>

#include <aperture/assert.hpp>
#include <aperture/device.hpp>
#include <aperture/log.hpp>
#include <aperture/queue.hpp>
#include <aperture/result.hpp>
#include <aperture/types.hpp>

#ifdef APERTURE_HAS_VULKAN
#include <aperture/vulkan/command/buffer.hpp>
#include <aperture/vulkan/device.hpp>
#include <aperture/vulkan/queue.hpp>
#include <aperture/vulkan/sync.hpp>
#endif

#include <cstdint>
#include <expected>

namespace aperture {

namespace {

[[nodiscard]] constexpr Backend BackendOfPtr(const void* ptr) noexcept {
  APERTURE_ASSERT(ptr != nullptr);
  return *static_cast<const Backend*>(ptr);
}

}  // namespace

auto CreateTimeline(Device device, uint64_t initial) noexcept
    -> Result<Timeline> {
  APERTURE_ASSERT(device.ptr != nullptr);

  switch (BackendOf(device)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      auto created =
          vk::CreateTimeline(static_cast<vk::Device*>(device.ptr), initial);
      if (!created) [[unlikely]] {
        return std::unexpected(created.error());
      }
      return Timeline{.ptr = *created};
    }
#endif
    default:
      log::Error("Unsupported backend: {} ({})!", ToString(BackendOf(device)),
                 ToString(Error::Unsupported));
      return std::unexpected(Error::Unsupported);
  }
}

void Wait(Timeline timeline, uint64_t value) noexcept {
  APERTURE_ASSERT(timeline.ptr != nullptr);

  switch (BackendOfPtr(timeline.ptr)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan:
      vk::Wait(static_cast<vk::Timeline*>(timeline.ptr), value);
      return;
#endif
    default:
      APERTURE_ASSERT(false, "Unsupported backend: {}!",
                      ToString(BackendOfPtr(timeline.ptr)));
  }
}

auto Wait(Timeline timeline, uint64_t value, uint64_t timeout_ns) noexcept
    -> Result<void> {
  APERTURE_ASSERT(timeline.ptr != nullptr);

  switch (BackendOfPtr(timeline.ptr)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan:
      return vk::Wait(static_cast<vk::Timeline*>(timeline.ptr), value,
                      timeout_ns);
#endif
    default:
      log::Error("Unsupported backend: {} ({})!",
                 ToString(BackendOfPtr(timeline.ptr)),
                 ToString(Error::Unsupported));
      return std::unexpected(Error::Unsupported);
  }
}

uint64_t CurrentValue(Timeline timeline) noexcept {
  APERTURE_ASSERT(timeline.ptr != nullptr);

  switch (BackendOfPtr(timeline.ptr)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan:
      return vk::CurrentValue(static_cast<vk::Timeline*>(timeline.ptr));
#endif
    default:
      APERTURE_ASSERT(false, "Unsupported backend: {}!",
                      ToString(BackendOfPtr(timeline.ptr)));
      return 0;
  }
}

void Destroy(Timeline timeline) noexcept {
  if (!timeline) {
    return;
  }

  switch (BackendOfPtr(timeline.ptr)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan:
      vk::Destroy(static_cast<vk::Timeline*>(timeline.ptr));
      return;
#endif
    default:
      APERTURE_ASSERT(false, "Unsupported backend: {}!",
                      ToString(BackendOfPtr(timeline.ptr)));
  }
}

auto Submit(Queue queue, const SubmitDesc& desc) noexcept -> Result<void> {
  APERTURE_ASSERT(queue.ptr != nullptr);

#ifdef APERTURE_HAS_VULKAN
  auto* vk_queue = static_cast<vk::Queue*>(queue.ptr);
  APERTURE_ASSERT(vk_queue->device != nullptr);
  switch (vk_queue->device->backend) {
    using enum Backend;
    case Vulkan:
      return vk::Submit(vk_queue, desc);
    default:
      break;
  }
  const Backend backend = vk_queue->device->backend;
#else
  const auto backend = Backend::Vulkan;
#endif
  log::Error("Unsupported backend: {} ({})!", ToString(backend),
             ToString(Error::Unsupported));
  return std::unexpected(Error::Unsupported);
}

}  // namespace aperture
