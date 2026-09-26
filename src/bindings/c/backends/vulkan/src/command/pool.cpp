#include <aperture/vulkan/command/pool.h>

#include <aperture/command/pool.h>
#include <aperture/command/pool.hpp>
#include <aperture/vulkan/command/pool.hpp>

extern "C" {

VkCommandPool aperture_vk_command_pool(ApertureCommandPool pool) noexcept {
  return aperture::vk::GetNative(aperture::CommandPool{.ptr = pool}).pool;
}
}
