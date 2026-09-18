#include <pch.hpp>

#include <aperture/vulkan/surface.hpp>

#include <aperture/assert.hpp>
#include <aperture/result.hpp>
#include <aperture/surface.hpp>
#include <aperture/vulkan/instance.hpp>
#include "internal.hpp"

#include <aperture/vulkan/header.hpp>

#include <concepts>
#include <expected>
#include <type_traits>
#include <variant>

namespace aperture::vk {

auto CreateSurface(Instance* instance,
                   const aperture::Surface& surface) noexcept
    -> Result<Surface> {
  APERTURE_ASSERT(instance != nullptr);
  APERTURE_ASSERT(instance->instance != VK_NULL_HANDLE);

  return std::visit(
      [instance](const auto& spec) noexcept -> Result<Surface> {
        using T = std::decay_t<decltype(spec)>;
        const bool log = instance->log_failed_results;
        VkInstance vk_instance = instance->instance;
        VkSurfaceKHR out = VK_NULL_HANDLE;

        if constexpr (std::same_as<T, Win32Surface>) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
          if (vkCreateWin32SurfaceKHR == nullptr) [[unlikely]] {
            LogFailed(log, "vkCreateWin32SurfaceKHR is unavailable ({})!",
                      ToString(Error::Unsupported));
            return std::unexpected(Error::Unsupported);
          }
          VkWin32SurfaceCreateInfoKHR ci{
              .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
              .hinstance = static_cast<HINSTANCE>(spec.hinstance),
              .hwnd = static_cast<HWND>(spec.hwnd),
          };
          const VkResult created =
              vkCreateWin32SurfaceKHR(vk_instance, &ci, nullptr, &out);
          if (created != VK_SUCCESS) [[unlikely]] {
            LogFailed(log, "vkCreateWin32SurfaceKHR failed: {}!",
                      ToString(created));
            return std::unexpected(MapVkResult(created));
          }
          return Surface{.surface = out};
#else
          LogFailed(log, "Platform surface is unsupported ({})!",
                    ToString(Error::Unsupported));
          return std::unexpected(Error::Unsupported);
#endif
        } else if constexpr (std::same_as<T, XlibSurface>) {
#ifdef VK_USE_PLATFORM_XLIB_KHR
          if (vkCreateXlibSurfaceKHR == nullptr) [[unlikely]] {
            LogFailed(log, "vkCreateXlibSurfaceKHR is unavailable ({})!",
                      ToString(Error::Unsupported));
            return std::unexpected(Error::Unsupported);
          }
          VkXlibSurfaceCreateInfoKHR ci{
              .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
              .dpy = static_cast<Display*>(spec.display),
              .window = spec.window,
          };
          const VkResult created =
              vkCreateXlibSurfaceKHR(vk_instance, &ci, nullptr, &out);
          if (created != VK_SUCCESS) [[unlikely]] {
            LogFailed(log, "vkCreateXlibSurfaceKHR failed: {}!",
                      ToString(created));
            return std::unexpected(MapVkResult(created));
          }
          return Surface{.surface = out};
#else
          LogFailed(log, "Platform surface is unsupported ({})!",
                    ToString(Error::Unsupported));
          return std::unexpected(Error::Unsupported);
#endif
        } else if constexpr (std::same_as<T, XcbSurface>) {
#ifdef VK_USE_PLATFORM_XCB_KHR
          if (vkCreateXcbSurfaceKHR == nullptr) [[unlikely]] {
            LogFailed(log, "vkCreateXcbSurfaceKHR is unavailable ({})!",
                      ToString(Error::Unsupported));
            return std::unexpected(Error::Unsupported);
          }
          VkXcbSurfaceCreateInfoKHR ci{
              .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
              .connection = static_cast<xcb_connection_t*>(spec.connection),
              .window = spec.window,
          };
          const VkResult created =
              vkCreateXcbSurfaceKHR(vk_instance, &ci, nullptr, &out);
          if (created != VK_SUCCESS) [[unlikely]] {
            LogFailed(log, "vkCreateXcbSurfaceKHR failed: {}!",
                      ToString(created));
            return std::unexpected(MapVkResult(created));
          }
          return Surface{.surface = out};
#else
          LogFailed(log, "Platform surface is unsupported ({})!",
                    ToString(Error::Unsupported));
          return std::unexpected(Error::Unsupported);
#endif
        } else if constexpr (std::same_as<T, WaylandSurface>) {
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
          if (vkCreateWaylandSurfaceKHR == nullptr) [[unlikely]] {
            LogFailed(log, "vkCreateWaylandSurfaceKHR is unavailable ({})!",
                      ToString(Error::Unsupported));
            return std::unexpected(Error::Unsupported);
          }
          VkWaylandSurfaceCreateInfoKHR ci{
              .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
              .display = static_cast<wl_display*>(spec.display),
              .surface = static_cast<wl_surface*>(spec.surface),
          };
          const VkResult created =
              vkCreateWaylandSurfaceKHR(vk_instance, &ci, nullptr, &out);
          if (created != VK_SUCCESS) [[unlikely]] {
            LogFailed(log, "vkCreateWaylandSurfaceKHR failed: {}!",
                      ToString(created));
            return std::unexpected(MapVkResult(created));
          }
          return Surface{.surface = out};
#else
          LogFailed(log, "Platform surface is unsupported ({})!",
                    ToString(Error::Unsupported));
          return std::unexpected(Error::Unsupported);
#endif
        } else if constexpr (std::same_as<T, AndroidSurface>) {
#ifdef VK_USE_PLATFORM_ANDROID_KHR
          if (vkCreateAndroidSurfaceKHR == nullptr) [[unlikely]] {
            LogFailed(log, "vkCreateAndroidSurfaceKHR is unavailable ({})!",
                      ToString(Error::Unsupported));
            return std::unexpected(Error::Unsupported);
          }
          VkAndroidSurfaceCreateInfoKHR ci{
              .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
              .window = static_cast<ANativeWindow*>(spec.window),
          };
          const VkResult created =
              vkCreateAndroidSurfaceKHR(vk_instance, &ci, nullptr, &out);
          if (created != VK_SUCCESS) [[unlikely]] {
            LogFailed(log, "vkCreateAndroidSurfaceKHR failed: {}!",
                      ToString(created));
            return std::unexpected(MapVkResult(created));
          }
          return Surface{.surface = out};
#else
          LogFailed(log, "Platform surface is unsupported ({})!",
                    ToString(Error::Unsupported));
          return std::unexpected(Error::Unsupported);
#endif
        } else {
          LogFailed(log, "Platform surface is unsupported ({})!",
                    ToString(Error::Unsupported));
          return std::unexpected(Error::Unsupported);
        }
      },
      surface.platform_surface);
}

void DestroySurface(Instance* instance, Surface surface) noexcept {
  APERTURE_ASSERT(instance != nullptr);
  if (surface.surface == VK_NULL_HANDLE || vkDestroySurfaceKHR == nullptr) {
    return;
  }
  vkDestroySurfaceKHR(instance->instance, surface.surface, nullptr);
}

}  // namespace aperture::vk
