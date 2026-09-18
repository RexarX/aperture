#ifndef APERTURE_H
#define APERTURE_H

#ifdef __cplusplus
extern "C" {
#endif

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
APERTURE_C_API const char* aperture_version(void) noexcept;
#else
APERTURE_C_API const char* aperture_version(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
