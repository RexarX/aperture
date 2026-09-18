#pragma once

#include <aperture/instance.hpp>
#include <aperture/result.hpp>
#include <aperture/types.hpp>
#include <aperture/vulkan/adapter.hpp>

#include <aperture/vulkan/header.hpp>

#include <span>
#include <string_view>
#include <vector>

namespace aperture::vk {

/// @brief Additive instance layers and extensions. Cannot disable Filter.
struct InstanceExtras {
  std::span<const char* const> layers;
  std::span<const char* const> extensions;
};

/// @brief Vulkan instance object.
/// @details First field is `Backend` so `BackendOf` can read it without
/// knowing the type. Non-copyable: `GetNative` returns a live reference.
struct Instance {
  Backend backend = Backend::Vulkan;
  bool log_failed_results = true;
  bool validation_fatal = true;
  VkInstance instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
  std::vector<Adapter> records;
  std::vector<aperture::Adapter> adapters;
};

/// @brief Creates a Vulkan instance. Declared without a `pNext` extras chain.
/// @param desc Instance creation parameters
/// @return The instance, or a recoverable `Error`
[[nodiscard]] APERTURE_API auto CreateInstance(
    const InstanceDesc& desc) noexcept -> Result<Instance*>;

/// @brief Creates a Vulkan instance with extra layers / extensions.
/// @details Empty extras is equivalent to `vk::CreateInstance(desc)`.
/// @param desc Instance creation parameters
/// @param extras Extra layers and extensions
/// @return The instance, or a recoverable `Error`
[[nodiscard]] APERTURE_API auto CreateInstance(
    const InstanceDesc& desc, const InstanceExtras& extras) noexcept
    -> Result<Instance*>;

/// @brief Destroys a Vulkan instance.
/// @param instance Instance to destroy
APERTURE_API void Destroy(Instance* instance) noexcept;

/// @brief Live Vulkan instance object for `instance`.
/// @param instance Portable instance handle
/// @return Reference valid until `Destroy(instance)`
/// @warning Asserts if `instance` is null or not a Vulkan instance.
[[nodiscard]] APERTURE_API Instance& GetNative(
    aperture::Instance instance) noexcept;

/// @brief Adapter records enumerated at Vulkan instance creation.
/// @param instance Instance that owns the adapter list
/// @return Span valid until `Destroy(instance)`
[[nodiscard]] APERTURE_API inline auto Adapters(
    const Instance& instance) noexcept -> std::span<const Adapter> {
  return instance.records;
}

/// @brief Portable Filter records enumerated at Vulkan instance creation.
/// @param instance Instance that owns the adapter list
/// @return Span valid until `Destroy(instance)`
[[nodiscard]] APERTURE_API inline auto AdapterInfos(
    const Instance& instance) noexcept -> std::span<const aperture::Adapter> {
  return instance.adapters;
}

/// @brief Instance extensions enumerated by the loader.
/// @return Span of instance extension properties
[[nodiscard]] APERTURE_API auto InstanceExtensions() noexcept
    -> std::span<const VkExtensionProperties>;

/// @brief Instance layers enumerated by the loader.
/// @return Span of instance layer properties
[[nodiscard]] APERTURE_API auto InstanceLayers() noexcept
    -> std::span<const VkLayerProperties>;

/// @brief True if `name` is in `InstanceExtensions()`.
/// @param name Extension name to look up
/// @return `true` if the instance extension is present
[[nodiscard]] APERTURE_API bool HasInstanceExtension(
    std::string_view name) noexcept;

}  // namespace aperture::vk
