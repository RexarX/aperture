# Provide `aperture::lib::volk`.
#
# Default resolution:
#   1. A compatible target already in the parent project
#   2. Vulkan SDK when APERTURE_USE_VULKAN_SDK_VOLK is ON and usable
#   3. find_package(volk) with version >= APERTURE_VOLK_VERSION
#   4. CPM fetch of zeux/volk
#
# APERTURE_USE_SYSTEM_VOLK skips the SDK and requires step 3.
#
# Static volk loader compiled against aperture::lib::vulkan_headers.

include_guard(GLOBAL)
include(CPM)
include(PackageResolve)
include(VulkanHeaders)

function(_aperture_accept_volk_dir DIR VERSION OUT_VAR)
  set(${OUT_VAR} FALSE PARENT_SCOPE)
  if(NOT DIR)
    return()
  endif()
  _aperture_is_sdk_path("${DIR}" _from_sdk)
  if(_from_sdk AND NOT APERTURE_USE_VULKAN_SDK_VOLK)
    return()
  endif()
  if(VERSION)
    _aperture_version_meets("${VERSION}" "${APERTURE_VOLK_VERSION}" _ok)
    if(NOT _ok)
      message(STATUS
          "volk: ${VERSION} at '${DIR}' is older than ${APERTURE_VOLK_VERSION}"
      )
      return()
    endif()
  endif()
  set(${OUT_VAR} TRUE PARENT_SCOPE)
endfunction()

function(_aperture_try_find_volk)
  find_package(volk ${APERTURE_VOLK_VERSION} CONFIG QUIET)
  set(_ver "${volk_VERSION}")
  if(TARGET volk::volk)
    _aperture_include_dir_from_target(volk::volk "volk.h" _dir)
    _aperture_accept_volk_dir("${_dir}" "${_ver}" _ok)
    if(_ok)
      set(_aperture_volk_dir "${_dir}" PARENT_SCOPE)
      set(_aperture_volk_source "package" PARENT_SCOPE)
      return()
    endif()
  endif()
  if(TARGET volk::volk_headers)
    _aperture_include_dir_from_target(volk::volk_headers "volk.h" _dir)
    _aperture_accept_volk_dir("${_dir}" "${_ver}" _ok)
    if(_ok)
      set(_aperture_volk_dir "${_dir}" PARENT_SCOPE)
      set(_aperture_volk_source "package" PARENT_SCOPE)
    endif()
  endif()
endfunction()

function(_aperture_fetch_volk)
  message(STATUS "volk: fetching ${APERTURE_VOLK_VERSION} via CPM")
  CPMAddPackage(
      NAME volk
      GITHUB_REPOSITORY zeux/volk
      GIT_TAG "vulkan-sdk-${APERTURE_VOLK_VERSION}"
      VERSION "${APERTURE_VOLK_VERSION}"
      GIT_SHALLOW TRUE
      DOWNLOAD_ONLY YES
  )
  if(volk_SOURCE_DIR AND EXISTS "${volk_SOURCE_DIR}/volk.h")
    set(_aperture_volk_dir "${volk_SOURCE_DIR}" PARENT_SCOPE)
    set(_aperture_volk_source "fetch" PARENT_SCOPE)
  endif()
endfunction()

if(TARGET aperture::lib::volk)
  return()
endif()

set(_aperture_volk_dir "")
set(_aperture_volk_source "")

_aperture_include_dir_from_target(volk::volk "volk.h" _aperture_volk_dir)
if(NOT _aperture_volk_dir)
  _aperture_include_dir_from_target(volk::volk_headers "volk.h" _aperture_volk_dir)
endif()
if(_aperture_volk_dir)
  set(_aperture_volk_source "provided")
endif()

if(NOT _aperture_volk_dir AND APERTURE_USE_VULKAN_SDK_VOLK AND APERTURE_VULKAN_SDK_USABLE)
  _aperture_find_header_dir(_aperture_volk_dir "volk.h"
      "${APERTURE_VULKAN_SDK_INCLUDE_DIR}/Volk"
      "${APERTURE_VULKAN_SDK_INCLUDE_DIR}/volk"
      "${APERTURE_VULKAN_SDK_INCLUDE_DIR}"
  )
  if(_aperture_volk_dir)
    set(_aperture_volk_source "sdk")
  else()
    message(STATUS
        "volk: SDK at '${APERTURE_VULKAN_SDK_DIR}' has no volk.h; falling back"
    )
  endif()
endif()

if(NOT _aperture_volk_dir)
  _aperture_try_find_volk()
  if(NOT _aperture_volk_dir AND APERTURE_USE_SYSTEM_VOLK)
    message(FATAL_ERROR
        "APERTURE_USE_SYSTEM_VOLK is ON but find_package(volk) did not provide "
        "volk.h ${APERTURE_VOLK_VERSION}+."
    )
  endif()
endif()

if(NOT _aperture_volk_dir AND NOT APERTURE_USE_SYSTEM_VOLK)
  _aperture_fetch_volk()
endif()

if(NOT _aperture_volk_dir)
  message(FATAL_ERROR
      "Could not locate volk.h ${APERTURE_VOLK_VERSION}+. Install a full "
      "Vulkan SDK, provide volk via find_package, or allow CPM to fetch zeux/volk."
  )
endif()

set(APERTURE_VOLK_INCLUDE_DIR "${_aperture_volk_dir}" CACHE INTERNAL "volk include directory")

add_library(aperture_volk STATIC
    "${PROJECT_SOURCE_DIR}/third-party/vulkan/volk.c"
)
add_library(aperture::lib::volk ALIAS aperture_volk)
set_target_properties(aperture_volk PROPERTIES
    OUTPUT_NAME aperture_volk
    EXPORT_NAME lib::volk
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    FOLDER "third-party"
)
target_include_directories(aperture_volk PUBLIC
    $<BUILD_INTERFACE:${APERTURE_VOLK_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(aperture_volk PUBLIC aperture::lib::vulkan_headers)
if(UNIX)
  target_link_libraries(aperture_volk PUBLIC ${CMAKE_DL_LIBS})
endif()
target_compile_options(aperture_volk PRIVATE
    $<$<C_COMPILER_ID:MSVC>:/W0>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-w>
)
if(APERTURE_BUILD_SHARED)
  set_target_properties(aperture_volk PROPERTIES
      C_VISIBILITY_PRESET hidden
      VISIBILITY_INLINES_HIDDEN ON
  )
endif()

message(STATUS
    "volk: aperture::lib::volk ready (${_aperture_volk_source}) -> ${_aperture_volk_dir}"
)
