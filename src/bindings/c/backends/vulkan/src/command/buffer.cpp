#include <aperture/vulkan/command/buffer.h>

#include <aperture/command/buffer.h>
#include <aperture/command/buffer.hpp>
#include <aperture/vulkan/command/buffer.hpp>

extern "C" {

VkCommandBuffer aperture_vk_command_buffer(
    ApertureCommandBuffer buffer) noexcept {
  return aperture::vk::GetNative(aperture::CommandBuffer{.ptr = buffer}).buffer;
}
}
