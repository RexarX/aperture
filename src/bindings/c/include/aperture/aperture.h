#ifndef APERTURE_H
#define APERTURE_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(APERTURE_C_SHARED)
#  if defined(_WIN32)
#    if defined(APERTURE_C_EXPORTS)
#      define APERTURE_C_API __declspec(dllexport)
#    else
#      define APERTURE_C_API __declspec(dllimport)
#    endif
#  else
#    define APERTURE_C_API __attribute__((visibility("default")))
#  endif
#else
#  define APERTURE_C_API
#endif

APERTURE_C_API const char* aperture_version(void);

#ifdef __cplusplus
}
#endif

#endif
