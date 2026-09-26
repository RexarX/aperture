#include <aperture/vulkan/sync.h>

#include <aperture/sync.h>
#include <aperture/vulkan/sync.hpp>

extern "C" {

VkSemaphore aperture_vk_timeline(ApertureTimeline timeline) noexcept {
  return aperture::vk::GetNative(aperture::Timeline{.ptr = timeline}).semaphore;
}
}
