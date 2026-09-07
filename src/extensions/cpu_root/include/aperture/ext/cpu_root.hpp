#pragma once

#include <aperture/aperture.hpp>

/// @file
/// @brief CPU-copied roots for small, CPU-generated draw/dispatch data.
/// @details Link `aperture::ext::cpu_root`. Not a core `Draw*` overload — core
///          still passes GPU-resident root pointers.
///          Contract: `specs.md` §4.21, `api_design.md` §8.1.
///
/// Shaders include `<aperture/ext/cpu_root.slang>` and take `CpuRoot<T>`.
/// Mixing `CpuRoot<T>` with core `RootData<T>` in one entry point is rejected
/// at `CreateProgram`.
///
/// `T` must be trivially copyable, `sizeof(T) % 4 == 0`, and
/// `sizeof(T) <= Info(device).max_cpu_root_bytes`. The value is copied during
/// the call; it need not outlive the command buffer. One root is shared by
/// every active graphics stage. No `IndirectMulti`, no GPU-generated roots.

namespace aperture::ext::cpu_root {}
