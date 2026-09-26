#include <pch.hpp>

#include <aperture/vulkan/sync.hpp>

#include <aperture/assert.hpp>
#include <aperture/result.hpp>
#include <aperture/sync.hpp>
#include <aperture/types.hpp>

#include "command/state.hpp"
#include "internal.hpp"

#include <aperture/vulkan/header.hpp>

#include <cstdint>
#include <expected>
#include <limits>

namespace aperture::vk {

namespace {

constexpr void Unregister(CommandState* commands, Timeline* timeline) noexcept {
  APERTURE_ASSERT(commands != nullptr);
  std::erase(commands->timelines, timeline);
}

}  // namespace

auto CreateTimeline(Device* device, uint64_t initial) noexcept
    -> Result<Timeline*> {
  APERTURE_ASSERT(device != nullptr);
  APERTURE_ASSERT(device->commands != nullptr);
  const bool log = device->log_failed_results;

  const VkSemaphoreTypeCreateInfo type{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
      .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
      .initialValue = initial,
  };
  const VkSemaphoreCreateInfo create{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = &type,
  };

  auto* timeline = new Timeline{.device = device};
  const VkResult created =
      vkCreateSemaphore(device->device, &create, nullptr, &timeline->semaphore);
  if (created != VK_SUCCESS) [[unlikely]] {
    delete timeline;
    LogFailed(log, "vkCreateSemaphore failed: {} ({})!", ToString(created),
              ToString(MapVkResult(created)));
    return std::unexpected(MapVkResult(created));
  }

  device->commands->timelines.push_back(timeline);
  return timeline;
}

auto Wait(Timeline* timeline, uint64_t value, uint64_t timeout_ns) noexcept
    -> Result<void> {
  APERTURE_ASSERT(timeline != nullptr);
  APERTURE_ASSERT(timeline->device != nullptr);

  const VkSemaphoreWaitInfo wait{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
      .semaphoreCount = 1,
      .pSemaphores = &timeline->semaphore,
      .pValues = &value,
  };
  const VkResult waited =
      vkWaitSemaphores(timeline->device->device, &wait, timeout_ns);
  if (waited != VK_SUCCESS) [[unlikely]] {
    const Error error = MapVkResult(waited);
    LogFailed(timeline->device->log_failed_results,
              "vkWaitSemaphores failed: {} ({})!", ToString(waited),
              ToString(error));
    return std::unexpected(error);
  }
  return {};
}

void Wait(Timeline* timeline, uint64_t value) noexcept {
  const auto waited =
      Wait(timeline, value, std::numeric_limits<uint64_t>::max());
  APERTURE_VERIFY(waited.has_value(), "Timeline wait failed!");
}

uint64_t CurrentValue(Timeline* timeline) noexcept {
  APERTURE_ASSERT(timeline != nullptr);
  APERTURE_ASSERT(timeline->device != nullptr);

  uint64_t value = 0;
  const VkResult read = vkGetSemaphoreCounterValue(timeline->device->device,
                                                   timeline->semaphore, &value);
  APERTURE_VERIFY(read == VK_SUCCESS, "vkGetSemaphoreCounterValue failed!");
  return value;
}

void Destroy(Timeline* timeline) noexcept {
  if (timeline == nullptr) {
    return;
  }
  if (timeline->device != nullptr && timeline->device->commands != nullptr) {
    Unregister(timeline->device->commands, timeline);
    if (timeline->semaphore != VK_NULL_HANDLE) {
      vkDestroySemaphore(timeline->device->device, timeline->semaphore,
                         nullptr);
    }
  }
  delete timeline;
}

Timeline& GetNative(aperture::Timeline timeline) noexcept {
  APERTURE_ASSERT(timeline.ptr != nullptr);
  auto& impl = *static_cast<Timeline*>(timeline.ptr);
  APERTURE_ASSERT(impl.backend == Backend::Vulkan);
  return impl;
}

}  // namespace aperture::vk
