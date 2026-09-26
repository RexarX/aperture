#ifndef APERTURE_COMMAND_POOL_H
#define APERTURE_COMMAND_POOL_H

#include <aperture/platform.h>
#include <aperture/queue.h>
#include <aperture/result.h>

#include <stdint.h>

APERTURE_C_BEGIN

/// @brief Parameters for command-pool creation.
/// @details `max_timestamps` is the number of timestamp writes one buffer from
/// the pool may record. Zero disables timestamps.
typedef struct ApertureCommandPoolDesc {
  uint32_t max_timestamps;
} ApertureCommandPoolDesc;

/// @brief Pool that allocates command buffers. Invalid is `NULL`.
typedef struct ApertureCommandPoolImpl* ApertureCommandPool;

/// @brief Allocates a command pool for `queue`.
/// @param queue Queue the pool's buffers will be submitted to
/// @param desc Pool parameters. Null is the same as `max_timestamps == 0`
/// @param out Receives the pool on success
/// @return The creation error, or `APERTURE_ERROR_OK`
/// @warning Asserts in next cases:
/// - If `queue` is null
/// - If `out` is null
APERTURE_C_API ApertureError aperture_create_command_pool(
    ApertureQueue queue, const ApertureCommandPoolDesc* desc,
    ApertureCommandPool* out) APERTURE_C_NOEXCEPT;

/// @brief Recycles every buffer allocated from `pool`. Does not wait.
/// @param pool Pool to reset. Null is a no-op
/// @warning Asserts if a buffer from `pool` is still recording.
APERTURE_C_API void aperture_reset_command_pool(ApertureCommandPool pool)
    APERTURE_C_NOEXCEPT;

/// @brief Destroys a command pool. Does not wait on the GPU.
/// @param pool Pool to destroy. Null is a no-op
/// @warning Asserts if a buffer from `pool` is still recording.
APERTURE_C_API void aperture_destroy_command_pool(ApertureCommandPool pool)
    APERTURE_C_NOEXCEPT;

APERTURE_C_END

#endif
