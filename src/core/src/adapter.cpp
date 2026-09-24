#include <pch.hpp>

#include <aperture/adapter.hpp>

#include <aperture/format.hpp>
#include <aperture/log.hpp>
#include <aperture/surface.hpp>
#include <aperture/types.hpp>

#ifdef APERTURE_HAS_VULKAN
#include <aperture/vulkan/adapter.hpp>
#endif

#include <span>

namespace aperture {

bool SupportsFormat(const Adapter& adapter, TextureFormat format,
                    FormatUsage usage) noexcept {
  if (adapter.impl == nullptr) [[unlikely]] {
    return false;
  }

  auto backend = *static_cast<const Backend*>(adapter.impl);
  switch (backend) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      auto& vk_adapter = *static_cast<vk::Adapter*>(adapter.impl);
      return vk::SupportsFormat(vk_adapter, format, usage);
    }
#endif
    default:
      log::Error("Unsupported backend: {}!", ToString(backend));
      return false;
  }
}

auto PresentableFormats(const Adapter& adapter, const Surface& surface) noexcept
    -> std::span<const TextureFormat> {
  if (adapter.impl == nullptr) [[unlikely]] {
    return {};
  }

  auto backend = *static_cast<const Backend*>(adapter.impl);
  switch (backend) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      auto* vk_adapter = static_cast<vk::Adapter*>(adapter.impl);
      return vk::PresentableFormats(vk_adapter, surface);
    }
#endif
    default:
      log::Error("Unsupported backend: {}!", ToString(backend));
      return {};
  }
}

}  // namespace aperture
