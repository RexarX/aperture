#pragma once

#include <aperture/platform.hpp>
#include <aperture/result.hpp>
#include <aperture/sync.hpp>

#include <aperture/vulkan/header.hpp>

namespace aperture::vk {

struct Device;

/// @brief Vulkan timeline semaphore. `Backend` is first.
struct Timeline {
  Backend backend = Backend::Vulkan;
  Device* device = nullptr;
  VkSemaphore semaphore = VK_NULL_HANDLE;
};

/// @brief Creates a timeline semaphore.
/// @param device Owning device
/// @param initial Starting counter value
/// @return The timeline, or a recoverable `Error`
/// @warning Asserts if `device` is null.
[[nodiscard]] APERTURE_API auto CreateTimeline(Device* device,
                                               uint64_t initial) noexcept
    -> Result<Timeline*>;

/// @brief Blocks until `timeline` reaches `value`.
/// @param timeline Timeline to wait on
/// @param value Counter value
/// @warning Asserts in next cases:
/// - If `timeline` is null
/// - If the device is lost (`APERTURE_VERIFY`)
APERTURE_API void Wait(Timeline* timeline, uint64_t value) noexcept;

/// @brief Waits until `timeline` reaches `value`, or `timeout_ns` elapses.
/// @param timeline Timeline to wait on
/// @param value Counter value
/// @param timeout_ns Maximum wait in nanoseconds
/// @return Nothing, or `Error::Timeout` / `Error::DeviceLost`
/// @warning Asserts if `timeline` is null.
[[nodiscard]] APERTURE_API auto Wait(Timeline* timeline, uint64_t value,
                                     uint64_t timeout_ns) noexcept
    -> Result<void>;

/// @brief Latest signaled value. Does not block.
/// @param timeline Timeline to read
/// @return The current counter
/// @warning Asserts if `timeline` is null.
[[nodiscard]] APERTURE_API uint64_t CurrentValue(Timeline* timeline) noexcept;

/// @brief Destroys `timeline`. Does not wait.
/// @param timeline Timeline to destroy. Null is a no-op
APERTURE_API void Destroy(Timeline* timeline) noexcept;

/// @brief Live timeline for `timeline`.
/// @param timeline Portable timeline handle
/// @return Reference valid until `Destroy(timeline)`
/// @warning Asserts in next cases:
/// - If `timeline` is null
/// - If `timeline` is not Vulkan
[[nodiscard]] APERTURE_API Timeline& GetNative(
    aperture::Timeline timeline) noexcept;

}  // namespace aperture::vk
