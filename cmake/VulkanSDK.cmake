# Locate the Vulkan SDK (via the VULKAN_SDK environment variable, as set by
# LunarG's setup-env script / installer) and parse its header version.
#
# The SDK is optional. Packages use it only when APERTURE_USE_VULKAN_SDK is ON
# and the matching APERTURE_USE_VULKAN_SDK_* option is ON. A missing or too-old
# SDK is not fatal: dependents fall back to find_package / CPM.
#
# On success, sets in the including scope:
#   APERTURE_VULKAN_SDK_FOUND        - TRUE if the SDK root looks valid
#   APERTURE_VULKAN_SDK_USABLE       - TRUE if FOUND and version is new enough
#   APERTURE_VULKAN_SDK_DIR          - SDK root ($ENV{VULKAN_SDK})
#   APERTURE_VULKAN_SDK_VERSION      - e.g. "1.4.357"
#   APERTURE_VULKAN_SDK_INCLUDE_DIR  - .../Include or .../include
#   APERTURE_VULKAN_SDK_LIB_DIR      - .../Lib or .../lib
#   APERTURE_VULKAN_SDK_BIN_DIR      - .../Bin or .../bin

include_guard(GLOBAL)
include(PackageResolve)

function(_aperture_vulkan_sdk_clear)
  set(APERTURE_VULKAN_SDK_FOUND FALSE CACHE INTERNAL "Vulkan SDK located")
  set(APERTURE_VULKAN_SDK_USABLE FALSE CACHE INTERNAL "Vulkan SDK usable")
  set(APERTURE_VULKAN_SDK_DIR "" CACHE INTERNAL "Vulkan SDK root")
  set(APERTURE_VULKAN_SDK_VERSION "" CACHE INTERNAL "Vulkan SDK version")
  set(APERTURE_VULKAN_SDK_INCLUDE_DIR "" CACHE INTERNAL "Vulkan SDK include dir")
  set(APERTURE_VULKAN_SDK_LIB_DIR "" CACHE INTERNAL "Vulkan SDK lib dir")
  set(APERTURE_VULKAN_SDK_BIN_DIR "" CACHE INTERNAL "Vulkan SDK bin dir")
  set(APERTURE_VULKAN_SDK_FOUND FALSE PARENT_SCOPE)
  set(APERTURE_VULKAN_SDK_USABLE FALSE PARENT_SCOPE)
endfunction()

function(_aperture_find_vulkan_sdk)
  _aperture_vulkan_sdk_clear()

  if(NOT APERTURE_USE_VULKAN_SDK)
    message(STATUS "Vulkan SDK: APERTURE_USE_VULKAN_SDK is OFF")
    return()
  endif()

  if(NOT DEFINED ENV{VULKAN_SDK} OR "$ENV{VULKAN_SDK}" STREQUAL "")
    message(STATUS "Vulkan SDK: VULKAN_SDK environment variable is not set")
    return()
  endif()

  set(_sdk_dir "$ENV{VULKAN_SDK}")
  string(STRIP "${_sdk_dir}" _sdk_dir)
  if(NOT IS_DIRECTORY "${_sdk_dir}")
    message(STATUS "Vulkan SDK: VULKAN_SDK=\"${_sdk_dir}\" does not exist")
    return()
  endif()

  # SDK layout differs by platform: Windows uses "Include/Lib/Bin",
  # Linux/macOS LunarG tarballs use lowercase "include/lib/bin".
  if(EXISTS "${_sdk_dir}/Include/vulkan/vulkan_core.h")
    set(_include_dir "${_sdk_dir}/Include")
    set(_lib_dir "${_sdk_dir}/Lib")
    set(_bin_dir "${_sdk_dir}/Bin")
  elseif(EXISTS "${_sdk_dir}/include/vulkan/vulkan_core.h")
    set(_include_dir "${_sdk_dir}/include")
    set(_lib_dir "${_sdk_dir}/lib")
    set(_bin_dir "${_sdk_dir}/bin")
  else()
    message(STATUS "Vulkan SDK: could not find vulkan/vulkan_core.h under \"${_sdk_dir}\"")
    return()
  endif()

  _aperture_parse_vulkan_header_version("${_include_dir}" _version)
  if(NOT _version)
    message(STATUS "Vulkan SDK: could not parse VK_HEADER_VERSION from \"${_include_dir}/vulkan/vulkan_core.h\"")
    return()
  endif()
  message(STATUS "Vulkan SDK: found v${_version} at \"${_sdk_dir}\"")

  set(_usable TRUE)
  if(APERTURE_MIN_VULKAN_SDK_VERSION AND
      _version VERSION_LESS APERTURE_MIN_VULKAN_SDK_VERSION)
    message(WARNING
        "Vulkan SDK v${_version} at '${_sdk_dir}' is older than "
        "${APERTURE_MIN_VULKAN_SDK_VERSION}. SDK packages will not be used; "
        "dependents fall back to find_package / CPM."
    )
    set(_usable FALSE)
  endif()

  set(APERTURE_VULKAN_SDK_FOUND TRUE CACHE INTERNAL "Vulkan SDK located")
  set(APERTURE_VULKAN_SDK_USABLE "${_usable}" CACHE INTERNAL "Vulkan SDK usable")
  set(APERTURE_VULKAN_SDK_DIR "${_sdk_dir}" CACHE INTERNAL "Vulkan SDK root")
  set(APERTURE_VULKAN_SDK_VERSION "${_version}" CACHE INTERNAL "Vulkan SDK version")
  set(APERTURE_VULKAN_SDK_INCLUDE_DIR "${_include_dir}" CACHE INTERNAL "Vulkan SDK include dir")
  set(APERTURE_VULKAN_SDK_LIB_DIR "${_lib_dir}" CACHE INTERNAL "Vulkan SDK lib dir")
  set(APERTURE_VULKAN_SDK_BIN_DIR "${_bin_dir}" CACHE INTERNAL "Vulkan SDK bin dir")
  set(APERTURE_VULKAN_SDK_FOUND TRUE PARENT_SCOPE)
  set(APERTURE_VULKAN_SDK_USABLE "${_usable}" PARENT_SCOPE)
  set(APERTURE_VULKAN_SDK_DIR "${_sdk_dir}" PARENT_SCOPE)
  set(APERTURE_VULKAN_SDK_VERSION "${_version}" PARENT_SCOPE)
  set(APERTURE_VULKAN_SDK_INCLUDE_DIR "${_include_dir}" PARENT_SCOPE)
  set(APERTURE_VULKAN_SDK_LIB_DIR "${_lib_dir}" PARENT_SCOPE)
  set(APERTURE_VULKAN_SDK_BIN_DIR "${_bin_dir}" PARENT_SCOPE)
endfunction()

_aperture_find_vulkan_sdk()
