#pragma once

#include <aperture/device.hpp>
#include <aperture/platform.hpp>
#include <aperture/types.hpp>

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace aperture {

/// @brief Memory class of an allocation.
enum class Memory : uint8_t { Default, Readback };

/// @brief Name of a `Memory` enumerator.
/// @param memory Enumerator to convert
/// @return Enumerator name, or `"Unknown"`
[[nodiscard]] constexpr std::string_view ToString(Memory memory) noexcept {
  switch (memory) {
    using enum Memory;
    case Default:
      return "Default";
    case Readback:
      return "Readback";
  }
  return "Unknown";
}

/// @brief Cached device address of a host pointer from `Malloc`.
/// @param device Device that owns the allocation
/// @param host Host pointer returned by `Malloc`
/// @return Device address stored when the allocation was created
/// @warning Asserts if `device` or `host` is null, or if `host` is unknown.
/// @warning Same-`Device` heap ops are externally synchronized.
[[nodiscard]] APERTURE_API auto DeviceAddressOf(Device device,
                                                void* host) noexcept
    -> GpuPtr<std::byte>;

}  // namespace aperture
