#include <pch.hpp>

#include <aperture/instance.hpp>

#include <aperture/assert.hpp>
#include <aperture/log.hpp>
#include <aperture/result.hpp>
#include <aperture/types.hpp>
#include <aperture/version.hpp>

#ifdef APERTURE_HAS_VULKAN
#include <aperture/vulkan/instance.hpp>
#endif

#include <algorithm>
#include <expected>
#include <span>

namespace aperture {

auto CompiledBackends() noexcept -> std::span<const Backend> {
#ifdef APERTURE_HAS_VULKAN
  static constexpr Backend compiled[] = {Backend::Vulkan};
  return compiled;
#else
  return {};
#endif
}

auto AvailableBackends() noexcept -> std::span<const Backend> {
#if defined(APERTURE_HAS_VULKAN) && \
    (defined(_WIN32) || defined(__linux__) || defined(__ANDROID__))
  static constexpr Backend available[] = {Backend::Vulkan};
  return available;
#else
  return {};
#endif
}

bool Available(Backend backend) noexcept {
  return std::ranges::contains(AvailableBackends(), backend);
}

auto PreferredBackend() noexcept -> Result<Backend> {
  const auto available = AvailableBackends();
  if (available.empty()) [[unlikely]] {
    return std::unexpected(Error::Unsupported);
  }
  return available.front();
}

Backend BackendOf(Instance instance) noexcept {
  APERTURE_ASSERT(instance.ptr != nullptr);
  return *static_cast<const Backend*>(instance.ptr);
}

auto CreateInstance(const InstanceDesc& desc) noexcept -> Result<Instance> {
  auto preferred = PreferredBackend();
  if (!preferred) [[unlikely]] {
    if (desc.log_failed_results) {
      log::Error("No available backend ({})!", ToString(preferred.error()));
    }
    return std::unexpected(Error::Unsupported);
  }
  return CreateInstance(*preferred, desc);
}

auto CreateInstance(Backend backend, const InstanceDesc& desc) noexcept
    -> Result<Instance> {
  const Version linked = LinkedVersion();
  if (!Compatible(desc.header_version, linked)) [[unlikely]] {
    if (desc.log_failed_results) {
      log::Error(
          "Header version {}.{}.{} incompatible with linked {}.{}.{} "
          "({})!",
          desc.header_version.major, desc.header_version.minor,
          desc.header_version.patch, linked.major, linked.minor, linked.patch,
          ToString(Error::VersionMismatch));
    }
    return std::unexpected(Error::VersionMismatch);
  }

  if (!Available(backend)) [[unlikely]] {
    if (desc.log_failed_results) {
      log::Error("Backend '{}' is not available on this platform ({})!",
                 ToString(backend), ToString(Error::Unsupported));
    }
    return std::unexpected(Error::Unsupported);
  }

  switch (backend) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      auto created = vk::CreateInstance(desc);
      if (!created) [[unlikely]] {
        return std::unexpected(created.error());
      }
      return Instance{.ptr = *created};
    }
#endif
    case D3D12:
    case Metal:
    default:
      if (desc.log_failed_results) {
        log::Error("Backend '{}' is not compiled into this binary ({})!",
                   ToString(backend), ToString(Error::Unsupported));
      }
      return std::unexpected(Error::Unsupported);
  }
}

void Destroy(Instance instance) noexcept {
  if (!instance) [[unlikely]] {
    return;
  }

  switch (BackendOf(instance)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      auto* vk_instance = static_cast<vk::Instance*>(instance.ptr);
      vk::Destroy(vk_instance);
      return;
    }
#endif
    default:
      APERTURE_ASSERT(false, "Unknown instance backend: {}!",
                      ToString(BackendOf(instance)));
  }
}

auto Adapters(Instance instance) noexcept -> std::span<const Adapter> {
  APERTURE_ASSERT(instance.ptr != nullptr);

  switch (BackendOf(instance)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      const auto& vk_instance = *static_cast<const vk::Instance*>(instance.ptr);
      return vk::AdapterInfos(vk_instance);
    }
#endif
    default:
      return {};
  }
}

}  // namespace aperture
