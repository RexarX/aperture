#ifndef APERTURE_INSTANCE_H
#define APERTURE_INSTANCE_H

#include <aperture/adapter.h>
#include <aperture/platform.h>
#include <aperture/result.h>
#include <aperture/types.h>
#include <aperture/version.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

APERTURE_C_BEGIN

/// @brief Pointer-sized CPU handle for an instance. Invalid is `NULL`.
typedef struct ApertureInstanceImpl* ApertureInstance;

/// @brief Parameters for `aperture_create_instance`. Copied; the call does not
/// retain references.
/// @details `header_version` should be `aperture_header_version()` from the
/// caller's translation unit. Against a shared library this catches header /
/// DLL major–minor mismatches (`APERTURE_ERROR_VERSION_MISMATCH`).
typedef struct ApertureInstanceDesc {
  ApertureVersion header_version;
  bool log_failed_results;
  bool enable_validation;
  bool validation_fatal;
} ApertureInstanceDesc;

/// @brief Default instance desc with this TU's header version.
/// @return Desc with `header_version` set to `aperture_header_version()`
static inline ApertureInstanceDesc aperture_instance_desc(void) {
  const ApertureInstanceDesc desc = {
      .header_version = aperture_header_version(),
      .log_failed_results = true,
      .enable_validation = false,
      .validation_fatal = true,
  };
  return desc;
}

/// @brief Backends this binary was linked with.
/// @param data Receives pointer to the compiled backend array
/// @param size Receives element count
/// @warning Asserts if `data` or `size` is null.
APERTURE_C_API void aperture_compiled_backends(
    const ApertureBackend** data, size_t* size) APERTURE_C_NOEXCEPT;

/// @brief Compiled backends that are platform backends for this OS.
/// @param data Receives pointer to the available backend array
/// @param size Receives element count
/// @warning Asserts if `data` or `size` is null.
APERTURE_C_API void aperture_available_backends(
    const ApertureBackend** data, size_t* size) APERTURE_C_NOEXCEPT;

/// @brief Membership in `aperture_available_backends()`.
/// @param backend Backend to test
/// @return `true` if `backend` is available on this OS
APERTURE_C_API bool aperture_available(ApertureBackend backend)
    APERTURE_C_NOEXCEPT;

/// @brief First available backend.
/// @param out Receives the backend on success
/// @return `APERTURE_ERROR_OK`, or `APERTURE_ERROR_UNSUPPORTED` if none
/// @warning Asserts if `out` is null.
APERTURE_C_API ApertureError aperture_preferred_backend(ApertureBackend* out)
    APERTURE_C_NOEXCEPT;

/// @brief Native API of this instance.
/// @param instance Instance to query
/// @return Backend stored in the instance
/// @warning Asserts if `instance` is null.
APERTURE_C_API ApertureBackend
aperture_backend_of_instance(ApertureInstance instance) APERTURE_C_NOEXCEPT;

/// @brief Creates an instance using `aperture_preferred_backend()`.
/// @param desc Instance creation parameters
/// @param out Receives the instance on success
/// @return The instance error, or `APERTURE_ERROR_OK`
/// @warning Asserts if `desc` or `out` is null.
/// @warning No fallback: a missing backend is a hard failure.
APERTURE_C_API ApertureError
aperture_create_instance(const ApertureInstanceDesc* desc,
                         ApertureInstance* out) APERTURE_C_NOEXCEPT;

/// @brief Creates an instance for an explicit backend. No fallback.
/// @param backend Backend to create. Must be available
/// @param desc Instance creation parameters
/// @param out Receives the instance on success
/// @return The instance error, or `APERTURE_ERROR_OK`
/// @warning Asserts if `desc` or `out` is null.
APERTURE_C_API ApertureError aperture_create_instance_for(
    ApertureBackend backend, const ApertureInstanceDesc* desc,
    ApertureInstance* out) APERTURE_C_NOEXCEPT;

/// @brief Destroys an instance.
/// @details Must outlive every device created from it. Null is a no-op.
/// @param instance Instance to destroy
APERTURE_C_API void aperture_destroy_instance(ApertureInstance instance)
    APERTURE_C_NOEXCEPT;

/// @brief Number of adapters enumerated at instance creation.
/// @param instance Instance to query
/// @return Adapter count
/// @warning Asserts if `instance` is null.
APERTURE_C_API size_t aperture_adapter_count(ApertureInstance instance)
    APERTURE_C_NOEXCEPT;

/// @brief Adapter record at `index`. Pointers into strings are valid until
/// `aperture_destroy_instance`.
/// @param instance Instance that owns the adapter
/// @param index Adapter index in `[0, aperture_adapter_count(instance))`
/// @return Copy of the frozen adapter record
/// @warning Asserts if `instance` is null or `index` is out of range.
APERTURE_C_API ApertureAdapter
aperture_adapter(ApertureInstance instance, uint32_t index) APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
