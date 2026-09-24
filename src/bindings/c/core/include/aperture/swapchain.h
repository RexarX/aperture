#ifndef APERTURE_SWAPCHAIN_H
#define APERTURE_SWAPCHAIN_H

#include <aperture/platform.h>

#include <stdbool.h>
#include <stdint.h>

APERTURE_C_BEGIN

/// @brief Presentation modes reported by an adapter, as a bit mask.
typedef uint16_t AperturePresentMode;

enum {
  APERTURE_PRESENT_MODE_NONE = 0U,
  APERTURE_PRESENT_MODE_FIFO = 1U << 0U,
  APERTURE_PRESENT_MODE_IMMEDIATE = 1U << 1U,
  APERTURE_PRESENT_MODE_MAILBOX = 1U << 2U,
  APERTURE_PRESENT_MODE_WAITABLE = 1U << 3U,
};

/// @brief Name of an `AperturePresentMode` enumerator.
/// @param mode Exact enumerator or `NONE`. Combined masks return `"Flags"`.
/// @return Enumerator name, `"None"`, or `"Flags"`
static inline const char* aperture_present_mode_to_string(
    AperturePresentMode mode) {
  switch (mode) {
    case APERTURE_PRESENT_MODE_NONE:
      return "None";
    case APERTURE_PRESENT_MODE_FIFO:
      return "Fifo";
    case APERTURE_PRESENT_MODE_IMMEDIATE:
      return "Immediate";
    case APERTURE_PRESENT_MODE_MAILBOX:
      return "Mailbox";
    case APERTURE_PRESENT_MODE_WAITABLE:
      return "Waitable";
    default:
      return "Flags";
  }
}

/// @brief True if every bit in `bits` is set in `set`.
/// @param set Mask to test
/// @param bits Required bits
/// @return `true` if `(set & bits) == bits`
static inline bool aperture_present_mode_has_all(AperturePresentMode set,
                                                 AperturePresentMode bits) {
  return (AperturePresentMode)(set & bits) == bits;
}

/// @brief Pointer-sized CPU handle for a swapchain. Invalid is `NULL`.
typedef struct ApertureSwapchainImpl* ApertureSwapchain;

APERTURE_C_END

#endif
