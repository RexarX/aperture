# Provide `aperture::lib::vulkan_headers`.
#
# Default resolution:
#   1. A compatible target already in the parent project
#   2. Vulkan SDK when APERTURE_USE_VULKAN_SDK_VK_HEADERS is ON and usable
#   3. find_package with version >= APERTURE_VULKAN_HEADERS_VERSION
#   4. CPM fetch of KhronosGroup/Vulkan-Headers
#
# APERTURE_USE_SYSTEM_VULKAN_HEADERS skips the SDK and requires step 3.
#
# INTERFACE includes + platform defines, VK_NO_PROTOTYPES.

include_guard(GLOBAL)
include(CPM)
include(PackageResolve)

function(_aperture_accept_vulkan_headers_dir DIR OUT_VAR)
  set(${OUT_VAR} FALSE PARENT_SCOPE)
  if(NOT DIR)
    return()
  endif()
  _aperture_is_sdk_path("${DIR}" _from_sdk)
  if(_from_sdk AND NOT APERTURE_USE_VULKAN_SDK_VK_HEADERS)
    return()
  endif()
  _aperture_parse_vulkan_header_version("${DIR}" _ver)
  _aperture_version_meets("${_ver}" "${APERTURE_VULKAN_HEADERS_VERSION}" _ok)
  if(NOT _ok)
    if(_ver)
      message(STATUS
          "VulkanHeaders: ${_ver} at '${DIR}' is older than "
          "${APERTURE_VULKAN_HEADERS_VERSION}"
      )
    else()
      message(STATUS "VulkanHeaders: could not determine version at '${DIR}'")
    endif()
    return()
  endif()
  set(${OUT_VAR} TRUE PARENT_SCOPE)
endfunction()

function(_aperture_try_find_vulkan_headers)
  find_package(VulkanHeaders ${APERTURE_VULKAN_HEADERS_VERSION} CONFIG QUIET)
  if(TARGET Vulkan::Headers)
    _aperture_include_dir_from_target(Vulkan::Headers "vulkan/vulkan.h" _dir)
    _aperture_accept_vulkan_headers_dir("${_dir}" _ok)
    if(_ok)
      set(_aperture_vk_headers_dir "${_dir}" PARENT_SCOPE)
      set(_aperture_vk_headers_source "package" PARENT_SCOPE)
      return()
    endif()
  endif()

  find_package(Vulkan ${APERTURE_VULKAN_HEADERS_VERSION} QUIET)
  if(Vulkan_INCLUDE_DIR AND EXISTS "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.h")
    _aperture_accept_vulkan_headers_dir("${Vulkan_INCLUDE_DIR}" _ok)
    if(_ok)
      set(_aperture_vk_headers_dir "${Vulkan_INCLUDE_DIR}" PARENT_SCOPE)
      set(_aperture_vk_headers_source "package" PARENT_SCOPE)
    endif()
  endif()
endfunction()

function(_aperture_fetch_vulkan_headers)
  message(STATUS "VulkanHeaders: fetching v${APERTURE_VULKAN_HEADERS_VERSION} via CPM")
  CPMAddPackage(
      NAME VulkanHeaders
      GITHUB_REPOSITORY KhronosGroup/Vulkan-Headers
      GIT_TAG "v${APERTURE_VULKAN_HEADERS_VERSION}"
      VERSION "${APERTURE_VULKAN_HEADERS_VERSION}"
      GIT_SHALLOW TRUE
      DOWNLOAD_ONLY YES
  )
  if(VulkanHeaders_SOURCE_DIR AND
      EXISTS "${VulkanHeaders_SOURCE_DIR}/include/vulkan/vulkan.h")
    set(_aperture_vk_headers_dir "${VulkanHeaders_SOURCE_DIR}/include" PARENT_SCOPE)
    set(_aperture_vk_headers_source "fetch" PARENT_SCOPE)
  endif()
endfunction()

if(TARGET aperture::lib::vulkan_headers)
  return()
endif()

set(_aperture_vk_headers_dir "")
set(_aperture_vk_headers_source "")

_aperture_include_dir_from_target(Vulkan::Headers "vulkan/vulkan.h" _aperture_vk_headers_dir)
if(_aperture_vk_headers_dir)
  set(_aperture_vk_headers_source "provided")
endif()

if(NOT _aperture_vk_headers_dir AND APERTURE_USE_VULKAN_SDK_VK_HEADERS
    AND APERTURE_VULKAN_SDK_USABLE)
  _aperture_find_header_dir(_aperture_vk_headers_dir "vulkan/vulkan.h"
      "${APERTURE_VULKAN_SDK_INCLUDE_DIR}"
  )
  if(_aperture_vk_headers_dir)
    set(_aperture_vk_headers_source "sdk")
  else()
    message(STATUS
        "VulkanHeaders: SDK at '${APERTURE_VULKAN_SDK_DIR}' has no "
        "vulkan/vulkan.h; falling back"
    )
  endif()
endif()

if(NOT _aperture_vk_headers_dir)
  _aperture_try_find_vulkan_headers()
  if(NOT _aperture_vk_headers_dir AND APERTURE_USE_SYSTEM_VULKAN_HEADERS)
    message(FATAL_ERROR
        "APERTURE_USE_SYSTEM_VULKAN_HEADERS is ON but find_package could not "
        "locate Vulkan headers ${APERTURE_VULKAN_HEADERS_VERSION}+ "
        "(VulkanHeaders CONFIG or Vulkan)."
    )
  endif()
endif()

if(NOT _aperture_vk_headers_dir AND NOT APERTURE_USE_SYSTEM_VULKAN_HEADERS)
  _aperture_fetch_vulkan_headers()
endif()

if(NOT _aperture_vk_headers_dir)
  message(FATAL_ERROR
      "Could not locate Vulkan headers ${APERTURE_VULKAN_HEADERS_VERSION}+. "
      "Install a Vulkan SDK, provide them via find_package, or allow CPM to "
      "fetch KhronosGroup/Vulkan-Headers."
  )
endif()

set(APERTURE_VULKAN_HEADERS_INCLUDE_DIR "${_aperture_vk_headers_dir}" CACHE INTERNAL
    "Vulkan headers include directory"
)
set(APERTURE_VULKAN_HEADERS_SOURCE "${_aperture_vk_headers_source}" CACHE INTERNAL
    "Vulkan headers source (provided|package|sdk|fetch)"
)

set(_aperture_vk_defs VK_NO_PROTOTYPES)
if(WIN32)
  list(APPEND _aperture_vk_defs VK_USE_PLATFORM_WIN32_KHR)
endif()
if(APPLE)
  list(APPEND _aperture_vk_defs VK_USE_PLATFORM_METAL_EXT)
endif()
if(XLIB_HEADERS)
  list(APPEND _aperture_vk_defs VK_USE_PLATFORM_XLIB_KHR)
endif()
if(XCB_HEADERS)
  list(APPEND _aperture_vk_defs VK_USE_PLATFORM_XCB_KHR)
endif()
if(WAYLAND_HEADERS)
  list(APPEND _aperture_vk_defs VK_USE_PLATFORM_WAYLAND_KHR)
endif()

add_library(aperture_vulkan_headers INTERFACE)
add_library(aperture::lib::vulkan_headers ALIAS aperture_vulkan_headers)
set_target_properties(aperture_vulkan_headers PROPERTIES EXPORT_NAME lib::vulkan_headers)
target_include_directories(aperture_vulkan_headers INTERFACE
    $<BUILD_INTERFACE:${APERTURE_VULKAN_HEADERS_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_definitions(aperture_vulkan_headers INTERFACE ${_aperture_vk_defs})

message(STATUS
    "VulkanHeaders: aperture::lib::vulkan_headers ready "
    "(${_aperture_vk_headers_source}) -> ${_aperture_vk_headers_dir}"
)
