#include <pch.hpp>

#include <aperture/command/buffer.hpp>

#include <aperture/assert.hpp>
#include <aperture/command/pool.hpp>
#include <aperture/log.hpp>
#include <aperture/result.hpp>
#include <aperture/types.hpp>

#ifdef APERTURE_HAS_VULKAN
#include <aperture/vulkan/command/buffer.hpp>
#include <aperture/vulkan/command/pool.hpp>
#endif

#include <expected>

namespace aperture {

namespace {

[[nodiscard]] constexpr Backend BackendOfPtr(const void* ptr) noexcept {
  APERTURE_ASSERT(ptr != nullptr);
  return *static_cast<const Backend*>(ptr);
}

}  // namespace

auto Begin(CommandPool pool) noexcept -> Result<CommandBuffer> {
  APERTURE_ASSERT(pool.ptr != nullptr);

  switch (BackendOfPtr(pool.ptr)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan: {
      auto begun = vk::Begin(static_cast<vk::CommandPool*>(pool.ptr));
      if (!begun) [[unlikely]] {
        return std::unexpected(begun.error());
      }
      return CommandBuffer{.ptr = *begun};
    }
#endif
    default:
      log::Error("Unsupported backend: {} ({})!",
                 ToString(BackendOfPtr(pool.ptr)),
                 ToString(Error::Unsupported));
      return std::unexpected(Error::Unsupported);
  }
}

void Barrier(CommandBuffer* buffer, Stage src, Stage dst,
             Hazard hazards) noexcept {
  APERTURE_ASSERT(buffer != nullptr);
  APERTURE_ASSERT(buffer->ptr != nullptr);

  switch (BackendOfPtr(buffer->ptr)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan:
      vk::Barrier(static_cast<vk::CommandBuffer*>(buffer->ptr), src, dst,
                  hazards);
      return;
#endif
    default:
      APERTURE_ASSERT(false, "Unsupported backend: {}!",
                      ToString(BackendOfPtr(buffer->ptr)));
  }
}

void Copy(CommandBuffer* buffer, GpuRange dst, GpuRange src) noexcept {
  APERTURE_ASSERT(buffer != nullptr);
  APERTURE_ASSERT(buffer->ptr != nullptr);

  switch (BackendOfPtr(buffer->ptr)) {
    using enum Backend;
#ifdef APERTURE_HAS_VULKAN
    case Vulkan:
      vk::Copy(static_cast<vk::CommandBuffer*>(buffer->ptr), dst, src);
      return;
#endif
    default:
      APERTURE_ASSERT(false, "Unsupported backend: {}!",
                      ToString(BackendOfPtr(buffer->ptr)));
  }
}

}  // namespace aperture
