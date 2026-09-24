# Provide `aperture::lib::slang` as an alias of `slang::slang`.
#
# Default resolution:
#   1. A slang::slang target already in the parent project
#   2. Vulkan SDK when APERTURE_USE_VULKAN_SDK_SLANG is ON and the SDK is usable
#   3. find_package(slang) with version >= APERTURE_SLANG_VERSION
#   4. CPM-fetch the prebuilt GitHub release
#
# APERTURE_USE_SYSTEM_SLANG skips the SDK and requires step 3.
#
# Runtime DLLs/SOs still need to sit next to consumers; see SlangRuntime.cmake.

include_guard(GLOBAL)
include(CMakeDependentOption)
include(CPM)
include(PackageResolve)
include(SlangRuntime)

if(NOT APERTURE_SLANG_VERSION)
  set(APERTURE_SLANG_VERSION "2026.16.1" CACHE STRING
      "Slang version to fetch from GitHub releases if not found via find_package / Vulkan SDK"
  )
endif()

option(APERTURE_USE_SYSTEM_SLANG
    "Require slang from find_package (examples only)"
    OFF
)
cmake_dependent_option(APERTURE_USE_VULKAN_SDK_SLANG
    "Use Slang from the Vulkan SDK (examples only)"
    ON
    "APERTURE_USE_VULKAN_SDK;NOT APERTURE_USE_SYSTEM_SLANG"
    OFF
)

function(_aperture_slang_map_imported_configs)
  if(NOT TARGET slang::slang)
    return()
  endif()
  set_property(TARGET slang::slang PROPERTY IMPORTED_GLOBAL TRUE)
  foreach(_cfg IN ITEMS DEBUG RELWITHDEBINFO MINSIZEREL)
    set_property(TARGET slang::slang APPEND PROPERTY
        MAP_IMPORTED_CONFIG_${_cfg} Release
    )
  endforeach()
endfunction()

function(_aperture_slang_record_runtime)
  set(_dll "")
  get_target_property(_dll slang::slang IMPORTED_LOCATION_RELEASE)
  if(NOT _dll)
    get_target_property(_dll slang::slang IMPORTED_LOCATION)
  endif()
  if(NOT _dll)
    message(FATAL_ERROR "Slang: slang::slang has no IMPORTED_LOCATION")
  endif()
  get_filename_component(_runtime_dir "${_dll}" DIRECTORY)

  set(APERTURE_SLANG_RUNTIME_DIR "${_runtime_dir}" CACHE INTERNAL "Slang compiler runtime directory")
endfunction()

function(_aperture_slang_find_config)
  set(_found FALSE PARENT_SCOPE)
  set(_prefixes ${ARGN})
  if(NOT _prefixes)
    return()
  endif()

  find_package(slang CONFIG QUIET
      PATHS ${_prefixes}
      NO_DEFAULT_PATH
  )
  if(slang_FOUND AND TARGET slang::slang)
    set(_found TRUE PARENT_SCOPE)
    set(slang_FOUND TRUE PARENT_SCOPE)
    set(slang_VERSION "${slang_VERSION}" PARENT_SCOPE)
  endif()
endfunction()

# Vulkan SDK (and some distro layouts) ship the compiler without slangConfig.cmake.
# Recreate the same imported target the official package would: slang::slang.
function(_aperture_import_slang_from_prefix PREFIX)
  if(TARGET slang::slang)
    return()
  endif()

  set(_inc_dirs)
  set(_libdir "")
  set(_bindir "")

  if(EXISTS "${PREFIX}/include/slang.h")
    list(APPEND _inc_dirs "${PREFIX}/include")
    set(_libdir "${PREFIX}/lib")
    set(_bindir "${PREFIX}/bin")
  elseif(EXISTS "${PREFIX}/Include/slang/slang.h")
    list(APPEND _inc_dirs "${PREFIX}/Include" "${PREFIX}/Include/slang")
    set(_libdir "${PREFIX}/Lib")
    set(_bindir "${PREFIX}/Bin")
  elseif(EXISTS "${PREFIX}/include/slang/slang.h")
    list(APPEND _inc_dirs "${PREFIX}/include" "${PREFIX}/include/slang")
    set(_libdir "${PREFIX}/lib")
    set(_bindir "${PREFIX}/bin")
  else()
    return()
  endif()

  set(_dll "")
  set(_implib "")
  if(WIN32)
    if(EXISTS "${_bindir}/slang-compiler.dll")
      set(_dll "${_bindir}/slang-compiler.dll")
    elseif(EXISTS "${_bindir}/slang.dll")
      set(_dll "${_bindir}/slang.dll")
    endif()
    if(EXISTS "${_libdir}/slang-compiler.lib")
      set(_implib "${_libdir}/slang-compiler.lib")
    elseif(EXISTS "${_libdir}/slang.lib")
      set(_implib "${_libdir}/slang.lib")
    endif()
  else()
    set(_p "${CMAKE_SHARED_LIBRARY_PREFIX}")
    set(_s "${CMAKE_SHARED_LIBRARY_SUFFIX}")
    foreach(_name IN ITEMS slang-compiler slang)
      if(EXISTS "${_libdir}/${_p}${_name}${_s}")
        set(_dll "${_libdir}/${_p}${_name}${_s}")
        break()
      endif()
    endforeach()
  endif()

  if(NOT _dll)
    return()
  endif()
  if(WIN32 AND NOT _implib)
    return()
  endif()

  add_library(slang::slang SHARED IMPORTED GLOBAL)
  set_target_properties(slang::slang PROPERTIES
      IMPORTED_LOCATION "${_dll}"
      IMPORTED_LOCATION_RELEASE "${_dll}"
      INTERFACE_INCLUDE_DIRECTORIES "${_inc_dirs}"
      INTERFACE_COMPILE_DEFINITIONS "SLANG_DYNAMIC"
  )
  if(_implib)
    set_target_properties(slang::slang PROPERTIES
        IMPORTED_IMPLIB "${_implib}"
        IMPORTED_IMPLIB_RELEASE "${_implib}"
    )
  endif()

  set(_ver "")
  foreach(_ver_header IN ITEMS
      "${PREFIX}/include/slang-tag-version.h"
      "${PREFIX}/Include/slang/slang-tag-version.h"
      "${PREFIX}/include/slang/slang-tag-version.h"
  )
    if(EXISTS "${_ver_header}")
      file(STRINGS "${_ver_header}" _ver_line REGEX "^#define SLANG_VERSION_NUMERIC ")
      if(_ver_line)
        string(REGEX REPLACE "^#define SLANG_VERSION_NUMERIC \"([^\"]+)\"" "\\1" _ver "${_ver_line}")
      endif()
      break()
    endif()
  endforeach()

  if(_ver)
    message(STATUS "Slang: imported slang::slang from \"${PREFIX}\" (v${_ver})")
    set(slang_VERSION "${_ver}" PARENT_SCOPE)
  else()
    message(STATUS "Slang: imported slang::slang from \"${PREFIX}\"")
  endif()
  set(slang_FOUND TRUE PARENT_SCOPE)
endfunction()

function(_aperture_fetch_slang_prefix)
  if(WIN32)
    set(_platform "windows")
  elseif(APPLE)
    set(_platform "macos")
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(_platform "linux")
  else()
    message(FATAL_ERROR "Slang: no prebuilt binaries available for \"${CMAKE_SYSTEM_NAME}\"")
  endif()

  if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
    set(_arch "aarch64")
  else()
    set(_arch "x86_64")
  endif()

  set(_asset "slang-${APERTURE_SLANG_VERSION}-${_platform}-${_arch}")
  if(_platform STREQUAL "linux")
    if(_arch STREQUAL "aarch64")
      set(_glibc_default "2.28")
    else()
      set(_glibc_default "2.27")
    endif()
    set(APERTURE_SLANG_LINUX_GLIBC "${_glibc_default}" CACHE STRING
        "glibc version suffix for the fetched Slang Linux release (empty = unsuffixed asset)"
    )
    if(APERTURE_SLANG_LINUX_GLIBC)
      string(APPEND _asset "-glibc-${APERTURE_SLANG_LINUX_GLIBC}")
    endif()
  endif()
  string(APPEND _asset ".zip")

  set(_url "https://github.com/shader-slang/slang/releases/download/v${APERTURE_SLANG_VERSION}/${_asset}")
  message(STATUS "Slang: fetching prebuilt CMake package from ${_url}")

  CPMAddPackage(
      NAME slang_prebuilt
      URL "${_url}"
      DOWNLOAD_ONLY YES
  )

  set(slang_prebuilt_SOURCE_DIR "${slang_prebuilt_SOURCE_DIR}" PARENT_SCOPE)
endfunction()

function(aperture_setup_slang)
  if(TARGET aperture::lib::slang)
    return()
  endif()

  set(_aperture_slang_ready FALSE)

  if(TARGET slang::slang)
    set(_aperture_slang_ready TRUE)
  endif()

  if(NOT _aperture_slang_ready AND APERTURE_USE_VULKAN_SDK_SLANG
      AND APERTURE_VULKAN_SDK_USABLE)
    unset(slang_DIR CACHE)
    _aperture_slang_find_config("${APERTURE_VULKAN_SDK_DIR}")
    if(NOT _found)
      message(STATUS "Slang: Vulkan SDK has no slangConfig.cmake, importing binaries from the SDK")
      _aperture_import_slang_from_prefix("${APERTURE_VULKAN_SDK_DIR}")
      if(NOT TARGET slang::slang)
        message(STATUS "Slang: Vulkan SDK at \"${APERTURE_VULKAN_SDK_DIR}\" has no usable Slang binaries")
      endif()
    else()
      message(STATUS "Slang: found ${slang_VERSION} in Vulkan SDK via find_package")
    endif()
    if(TARGET slang::slang)
      set(_aperture_slang_ready TRUE)
    endif()
  endif()

  if(NOT _aperture_slang_ready)
    if(APERTURE_USE_SYSTEM_SLANG)
      find_package(slang ${APERTURE_SLANG_VERSION} CONFIG REQUIRED)
    else()
      find_package(slang ${APERTURE_SLANG_VERSION} CONFIG QUIET)
    endif()
    if(slang_FOUND AND TARGET slang::slang)
      set(_ok TRUE)
      if(slang_VERSION)
        _aperture_version_meets("${slang_VERSION}" "${APERTURE_SLANG_VERSION}" _ok)
      endif()
      if(_ok)
        set(_aperture_slang_ready TRUE)
        message(STATUS "Slang: found ${slang_VERSION} via find_package")
      elseif(APERTURE_USE_SYSTEM_SLANG)
        message(FATAL_ERROR
            "APERTURE_USE_SYSTEM_SLANG is ON but slang ${slang_VERSION} is older "
            "than ${APERTURE_SLANG_VERSION}."
        )
      else()
        message(STATUS
            "Slang: ignoring system slang '${slang_VERSION}' "
            "(need ${APERTURE_SLANG_VERSION}+)"
        )
      endif()
    elseif(APERTURE_USE_SYSTEM_SLANG)
      message(FATAL_ERROR
          "APERTURE_USE_SYSTEM_SLANG is ON but find_package(slang) did not "
          "provide slang::slang ${APERTURE_SLANG_VERSION}+."
      )
    endif()
  endif()

  if(NOT _aperture_slang_ready AND NOT APERTURE_USE_SYSTEM_SLANG)
    if(TARGET slang::slang)
      message(FATAL_ERROR
          "Slang: find_package created slang::slang ${slang_VERSION} which is "
          "not ${APERTURE_SLANG_VERSION}+. Uninstall it, or set "
          "APERTURE_USE_SYSTEM_SLANG=ON to use it anyway."
      )
    endif()
    _aperture_fetch_slang_prefix()
    _aperture_slang_find_config("${slang_prebuilt_SOURCE_DIR}")
    if(NOT TARGET slang::slang)
      _aperture_import_slang_from_prefix("${slang_prebuilt_SOURCE_DIR}")
    endif()
    if(TARGET slang::slang)
      set(_aperture_slang_ready TRUE)
      message(STATUS "Slang: using fetched package from \"${slang_prebuilt_SOURCE_DIR}\"")
    endif()
  endif()

  if(NOT _aperture_slang_ready OR NOT TARGET slang::slang)
    message(FATAL_ERROR
        "Slang: could not find slang::slang ${APERTURE_SLANG_VERSION}+. "
        "Install a Vulkan SDK with Slang, provide slang via find_package / "
        "CMAKE_PREFIX_PATH, or allow the GitHub prebuilt CPM fetch."
    )
  endif()

  _aperture_slang_map_imported_configs()
  _aperture_slang_record_runtime()

  add_library(aperture::lib::slang ALIAS slang::slang)

  message(STATUS "Slang: aperture::lib::slang -> slang::slang (runtime: ${APERTURE_SLANG_RUNTIME_DIR})")
endfunction()
