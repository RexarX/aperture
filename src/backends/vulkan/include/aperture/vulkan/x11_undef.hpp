#pragma once

// Xlib (transitively included by vulkan.h when VK_USE_PLATFORM_XLIB_KHR is set)
// defines C macros that collide with aperture enumerators (e.g. None).
#ifdef None
#undef None
#endif
