#ifndef APERTURE_EXT_CPU_ROOT_H
#define APERTURE_EXT_CPU_ROOT_H

/// @file
/// @brief CPU-copied roots for small, CPU-generated draw/dispatch data.
/// @details Shaders include `<aperture/ext/cpu_root.slang>` and take
/// `CpuRoot<T>`. Mixing `CpuRoot<T>` with core `RootData<T>` in one entry
/// point is a shader ABI contract; aperture does not inspect source.
///
/// `T` must be trivially copyable, `sizeof(T) % 4 == 0`, and
/// `sizeof(T) <= aperture_device_info(device).max_cpu_root_bytes`.

#endif
