#pragma once

#include <aperture/aperture.hpp>

/// @file
/// @brief CPU-copied roots for small, CPU-generated draw/dispatch data.
/// @details Shaders include `<aperture/ext/cpu_root.slang>` and take
/// `CpuRoot<T>`. Mixing `CpuRoot<T>` with core `RootData<T>` in one entry point
/// is rejected at `CreateProgram`.
///
/// `T` must be trivially copyable, `sizeof(T) % 4 == 0`, and
/// `sizeof(T) <= Info(device).max_cpu_root_bytes`.
/// The value is copied during the call (`vkCmdPushDataEXT` on Vulkan); it need
/// not outlive the command buffer.
/// One root is shared by every active graphics stage. No `IndirectMulti`, no
/// GPU-generated roots.

namespace aperture::ext::cpu_root {}
