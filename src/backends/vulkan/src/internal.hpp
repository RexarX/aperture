#pragma once

#include <aperture/assert.hpp>
#include <aperture/log.hpp>
#include <aperture/result.hpp>
#include <aperture/vulkan/device.hpp>
#include <aperture/vulkan/instance.hpp>
#include <aperture/vulkan/queue.hpp>

#include <aperture/vulkan/header.hpp>
#include <volk.h>

#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <memory_resource>
#include <span>
#include <string_view>
#include <utility>

#ifndef VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME
#define VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME \
  "VK_KHR_unified_image_layouts"
#endif
#ifndef VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME
#define VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME \
  "VK_EXT_device_generated_commands"
#endif
#ifndef VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME
#define VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME "VK_EXT_shader_tile_image"
#endif
#ifndef VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME
#define VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME \
  "VK_EXT_rasterization_order_attachment_access"
#endif

namespace aperture::vk {

inline constexpr size_t SCRATCH_BUFFER_SIZE = 4096;

struct Scratch {
  alignas(std::max_align_t) std::byte buffer[SCRATCH_BUFFER_SIZE]{};
  std::pmr::monotonic_buffer_resource resource{buffer, sizeof(buffer),
                                               std::pmr::new_delete_resource()};
};

[[nodiscard]] constexpr std::string_view ToString(VkResult result) noexcept {
  switch (result) {
    case VK_SUCCESS:
      return "VK_SUCCESS";
    case VK_NOT_READY:
      return "VK_NOT_READY";
    case VK_TIMEOUT:
      return "VK_TIMEOUT";
    case VK_EVENT_SET:
      return "VK_EVENT_SET";
    case VK_EVENT_RESET:
      return "VK_EVENT_RESET";
    case VK_INCOMPLETE:
      return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
      return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:
      return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:
      return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:
      return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:
      return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
      return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:
      return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
      return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:
      return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
      return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL:
      return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_UNKNOWN:
      return "VK_ERROR_UNKNOWN";
    case VK_ERROR_SURFACE_LOST_KHR:
      return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
      return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR:
      return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:
      return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
      return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT:
      return "VK_ERROR_VALIDATION_FAILED_EXT";
    default:
      return "VkResult";
  }
}

[[nodiscard]] constexpr Error MapVkResult(VkResult result) noexcept {
  switch (result) {
    case VK_SUCCESS:
      return Error::Ok;
    case VK_ERROR_OUT_OF_HOST_MEMORY:
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      return Error::OutOfMemory;
    case VK_ERROR_DEVICE_LOST:
      return Error::DeviceLost;
    case VK_ERROR_OUT_OF_DATE_KHR:
      return Error::OutOfDate;
    case VK_TIMEOUT:
      return Error::Timeout;
    default:
      return Error::Unsupported;
  }
}

inline void LogFailed(bool log, std::string_view message) noexcept {
  if (!log) {
    return;
  }
  log::Error(message);
}

template <typename... Args>
inline void LogFailed(bool log, std::format_string<Args...> fmt,
                      Args&&... args) noexcept {
  if (!log) {
    return;
  }
  log::Error(fmt, std::forward<Args>(args)...);
}

[[nodiscard]] constexpr bool HasExtension(
    std::span<const VkExtensionProperties> exts,
    std::string_view name) noexcept {
  for (const VkExtensionProperties& ext : exts) {
    if (ext.extensionName == name) {
      return true;
    }
  }
  return false;
}

inline constexpr const char* SURFACE_INSTANCE_EXTENSIONS[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#endif
};

template <typename Vector>
inline void AppendSurfaceInstanceExtensions(
    std::span<const VkExtensionProperties> available,
    Vector* extensions) noexcept {
  APERTURE_ASSERT(extensions != nullptr);
  for (const char* name : SURFACE_INSTANCE_EXTENSIONS) {
    if (HasExtension(available, name)) {
      extensions->push_back(name);
    }
  }
}

[[nodiscard]] inline bool PresentationSupport(
    [[maybe_unused]] VkPhysicalDevice physical,
    [[maybe_unused]] uint32_t family) noexcept {
#ifdef VK_USE_PLATFORM_WIN32_KHR
  if (vkGetPhysicalDeviceWin32PresentationSupportKHR != nullptr) {
    return vkGetPhysicalDeviceWin32PresentationSupportKHR(physical, family) ==
           VK_TRUE;
  }
#endif
  return true;
}

[[nodiscard]] constexpr auto ToU32(size_t count) noexcept -> Result<uint32_t> {
  if (count > std::numeric_limits<uint32_t>::max()) [[unlikely]] {
    return std::unexpected(Error::Invalid);
  }
  return static_cast<uint32_t>(count);
}

/// @brief Two-call Vulkan enumeration with `VK_INCOMPLETE` retry.
/// @param out Destination vector. Cleared on entry and on failure
/// @param query `VkResult(uint32_t* count, T* data)` callable
template <typename Vector, typename Query>
[[nodiscard]] auto EnumerateVk(Vector* out, Query&& query) noexcept
    -> Result<void> {
  APERTURE_ASSERT(out != nullptr);
  out->clear();
  while (true) {
    uint32_t count = 0;
    VkResult result = query(&count, nullptr);
    if (result != VK_SUCCESS && result != VK_INCOMPLETE) [[unlikely]] {
      return std::unexpected(MapVkResult(result));
    }

    if (count == 0) {
      return {};
    }
    out->resize(count);
    result = query(&count, out->data());
    if (result == VK_INCOMPLETE) {
      continue;
    }
    if (result != VK_SUCCESS) [[unlikely]] {
      out->clear();
      return std::unexpected(MapVkResult(result));
    }
    out->resize(count);
    return {};
  }
}

}  // namespace aperture::vk
