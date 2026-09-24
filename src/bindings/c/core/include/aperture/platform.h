#ifndef APERTURE_PLATFORM_H
#define APERTURE_PLATFORM_H

#ifdef APERTURE_BUILD_SHARED
#ifdef _WIN32
#ifdef APERTURE_EXPORTS
#define APERTURE_C_API __declspec(dllexport)
#else
#define APERTURE_C_API __declspec(dllimport)
#endif
#else
#define APERTURE_C_API __attribute__((visibility("default")))
#endif
#else
#define APERTURE_C_API
#endif

#ifdef __cplusplus
#define APERTURE_C_NOEXCEPT noexcept
#define APERTURE_C_BEGIN extern "C" {
#define APERTURE_C_END }
#define APERTURE_C_ALIGNAS(n) alignas(n)
#else
#define APERTURE_C_NOEXCEPT
#define APERTURE_C_BEGIN
#define APERTURE_C_END
#define APERTURE_C_ALIGNAS(n) _Alignas(n)
#endif

// MSVC: Use the intrinsic which maps to the appropriate inlined assembly
#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#define APERTURE_C_DEBUG_BREAK() __debugbreak()

// ARM64 on Apple: trap so CI/ctest terminate instead of swallowing SIGINT
#elif defined(__arm64__) && defined(__APPLE__)
#define APERTURE_C_DEBUG_BREAK() __builtin_debugtrap()

// ARM64 with GCC/Clang: Use brk instruction
#elif defined(__arm64__) && (defined(__GNUC__) || defined(__clang__))
#define APERTURE_C_DEBUG_BREAK() __asm__("brk 0")

// ARM (32-bit) on Apple: Use trap instruction
#elif defined(__arm__) && defined(__APPLE__)
#define APERTURE_C_DEBUG_BREAK() __asm__("trap")

// ARM (32-bit) with GCC/Clang: Use bkpt instruction
#elif defined(__arm__) && (defined(__GNUC__) || defined(__clang__))
#define APERTURE_C_DEBUG_BREAK() __asm__("bkpt 0")

// ARM with Arm Compiler: Use breakpoint intrinsic
#elif defined(__arm__) && defined(__ARMCC_VERSION)
#define APERTURE_C_DEBUG_BREAK() __breakpoint(0)

// x86/x86_64 with Clang/GCC: Use int3 instruction (AT&T syntax)
#elif (defined(__x86_64__) || defined(__i386__)) && \
    (defined(__GNUC__) || defined(__clang__))
#define APERTURE_C_DEBUG_BREAK() __asm__("int3")

// x86/x86_64 with MSVC: Use int3 instruction (Intel syntax)
#elif (defined(_M_X64) || defined(_M_IX86)) && defined(_MSC_VER)
#define APERTURE_C_DEBUG_BREAK() __asm int 3

// PowerPC: Trigger exception via opcode 0x00000000
#elif defined(__powerpc__) || defined(__ppc__) || defined(_ARCH_PPC)
#define APERTURE_C_DEBUG_BREAK() __asm__(".long 0")

// RISC-V: Use ebreak instruction
#elif defined(__riscv) || defined(__riscv__)
#define APERTURE_C_DEBUG_BREAK() __asm__("ebreak")

// WebAssembly: No breakpoint support, use unreachable
#elif defined(__wasm__)
#define APERTURE_C_DEBUG_BREAK() __asm__("unreachable")

// Fallback: Use compiler builtin trap (works on most modern compilers)
#elif defined(__has_builtin) && __has_builtin(__builtin_trap)
#define APERTURE_C_DEBUG_BREAK() __builtin_trap()

// Last resort: Use signal-based approach
#else
#include <signal.h>
#define APERTURE_C_DEBUG_BREAK() raise(SIGTRAP)
#endif

#endif
