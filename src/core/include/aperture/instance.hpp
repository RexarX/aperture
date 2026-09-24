#pragma once

#include <aperture/adapter.hpp>
#include <aperture/platform.hpp>
#include <aperture/result.hpp>
#include <aperture/types.hpp>
#include <aperture/version.hpp>

#include <span>

namespace aperture {

/// @brief Pointer-sized CPU handle for an instance.
struct Instance {
  void* ptr = nullptr;

  /// @brief True if this handle is non-null.
  /// @return `true` when `ptr != nullptr`
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return ptr != nullptr;
  }

  [[nodiscard]] constexpr bool operator==(const Instance&) const noexcept =
      default;
  [[nodiscard]] constexpr bool operator!=(const Instance&) const noexcept =
      default;
};

/// @brief Parameters for `CreateInstance`. Copied; the call does not retain
/// references.
/// @details `header_version` defaults to `HeaderVersion()` evaluated in the
/// caller's translation unit. Against a shared library this catches header /
/// DLL major–minor mismatches (`Error::VersionMismatch`).
///
/// Validation: `APERTURE_ENABLE_VALIDATION_SUPPORT` only compiles the
/// machinery in. Set `enable_validation` to turn it on for this instance.
/// When support was not compiled, `enable_validation == true` fails with
/// `Error::Unsupported`.
struct InstanceDesc {
  /// Headers this TU was compiled against (`HeaderVersion()` at the call site)
  Version header_version = HeaderVersion();
  /// Log recoverable failures through `aperture::log`
  bool log_failed_results = true;
  /// Enable validation (layers / messengers) when support was compiled in
  bool enable_validation = false;
  /// Abort on validation errors when validation is enabled
  bool validation_fatal = true;
};

/// @brief Backends this binary was linked with.
/// @return Span of compiled backends. Empty if none were linked
[[nodiscard]] APERTURE_API auto CompiledBackends() noexcept
    -> std::span<const Backend>;

/// @brief Compiled backends that are platform backends for this OS.
/// @return Span of backends that can run on this OS. Empty if none can
[[nodiscard]] APERTURE_API auto AvailableBackends() noexcept
    -> std::span<const Backend>;

/// @brief Membership in `AvailableBackends()`.
/// @param backend Backend to test
/// @return `true` if `backend` is in `AvailableBackends()`
[[nodiscard]] APERTURE_API bool Available(Backend backend) noexcept;

/// @brief First available backend.
/// @return `AvailableBackends()[0]`, or `Error::Unsupported` if the span is
/// empty
[[nodiscard]] APERTURE_API auto PreferredBackend() noexcept -> Result<Backend>;

/// @brief Native API of this instance.
/// @param instance Instance to query
/// @return Backend stored in the instance
/// @warning Asserts if `instance` is null.
[[nodiscard]] APERTURE_API Backend BackendOf(Instance instance) noexcept;

/// @brief Creates an instance using `PreferredBackend()`.
/// @details `desc` is copied. There is no fallback across backends.
/// @param desc Instance creation parameters
/// @return The instance, `Error::VersionMismatch` if `desc.header_version` is
/// incompatible with the linked binary, `Error::Unsupported` if no backend is
/// available or validation was requested without compiled support / without a
/// layer, or another recoverable `Error` from the backend
/// @warning No fallback: a missing backend is a hard failure.
[[nodiscard]] APERTURE_API auto CreateInstance(
    const InstanceDesc& desc) noexcept -> Result<Instance>;

/// @brief Creates an instance for an explicit backend. No fallback.
/// @param backend Backend to create. Must be in `AvailableBackends()`
/// @param desc Instance creation parameters
/// @return The instance, `Error::VersionMismatch` if `desc.header_version` is
/// incompatible with the linked binary, `Error::Unsupported` if `backend` is
/// unavailable or validation was requested without compiled support / without
/// a layer, or another recoverable `Error` from the backend
[[nodiscard]] APERTURE_API auto CreateInstance(
    Backend backend, const InstanceDesc& desc) noexcept -> Result<Instance>;

/// @brief Destroys an instance.
/// @details Must outlive every device created from it. Null is a no-op.
/// @param instance Instance to destroy
APERTURE_API void Destroy(Instance instance) noexcept;

/// @brief Adapters enumerated at instance creation.
/// @param instance Instance that owns the adapter list
/// @return Span valid until `Destroy(instance)`
/// @warning Asserts if `instance` is null.
[[nodiscard]] APERTURE_API auto Adapters(Instance instance) noexcept
    -> std::span<const Adapter>;

}  // namespace aperture
