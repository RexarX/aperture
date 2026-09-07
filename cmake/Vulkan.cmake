# Direct Vulkan stack for aperture: SDK headers + volk + VMA.
#
# Requires a usable Vulkan SDK that ships vulkan headers, volk, and VMA
# (the LunarG SDK does). Creates:
#   aperture::lib::vulkan_headers - SDK includes + platform defines, VK_NO_PROTOTYPES
#   aperture::lib::volk           - static volk loader
#   aperture::lib::vma            - static VMA (dynamic Vulkan functions via volk)
#
# These are linked by the Vulkan backend (src/backends/vulkan), not by core.

include_guard(GLOBAL)

if(NOT APERTURE_VULKAN_SDK_FOUND)
  message(FATAL_ERROR
      "Aperture requires the Vulkan SDK. Install LunarG Vulkan SDK "
      "${APERTURE_MIN_VULKAN_SDK_VERSION}+ and set the VULKAN_SDK environment "
      "variable (the installer does this on Windows; source setup-env.sh on "
      "Linux/macOS)."
  )
endif()

if(APERTURE_VULKAN_SDK_VERSION VERSION_LESS APERTURE_MIN_VULKAN_SDK_VERSION)
  message(FATAL_ERROR
      "Vulkan SDK v${APERTURE_VULKAN_SDK_VERSION} at '${APERTURE_VULKAN_SDK_DIR}' "
      "is too old. Aperture requires ${APERTURE_MIN_VULKAN_SDK_VERSION} or newer."
  )
endif()

function(_aperture_find_header_dir OUT_VAR FILENAME)
  foreach(_dir IN LISTS ARGN)
    if(_dir AND EXISTS "${_dir}/${FILENAME}")
      set(${OUT_VAR} "${_dir}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
  set(${OUT_VAR} "" PARENT_SCOPE)
endfunction()

_aperture_find_header_dir(_volk_inc "volk.h"
    "${APERTURE_VULKAN_SDK_INCLUDE_DIR}/Volk"
    "${APERTURE_VULKAN_SDK_INCLUDE_DIR}/volk"
    "${APERTURE_VULKAN_SDK_INCLUDE_DIR}"
)
if(NOT _volk_inc)
  message(FATAL_ERROR
      "Vulkan SDK at '${APERTURE_VULKAN_SDK_DIR}' has no volk.h. "
      "Install the full LunarG SDK (Include/Volk/volk.h)."
  )
endif()
message(STATUS "Vulkan: volk.h -> ${_volk_inc}")

_aperture_find_header_dir(_vma_inc "vk_mem_alloc.h"
    "${APERTURE_VULKAN_SDK_INCLUDE_DIR}/vma"
    "${APERTURE_VULKAN_SDK_INCLUDE_DIR}"
)
if(NOT _vma_inc)
  message(FATAL_ERROR
      "Vulkan SDK at '${APERTURE_VULKAN_SDK_DIR}' has no vk_mem_alloc.h. "
      "Install the full LunarG SDK (Include/vma/vk_mem_alloc.h)."
  )
endif()
message(STATUS "Vulkan: vk_mem_alloc.h -> ${_vma_inc}")

set(APERTURE_VOLK_INCLUDE_DIR "${_volk_inc}" CACHE INTERNAL "volk include directory")
set(APERTURE_VMA_INCLUDE_DIR "${_vma_inc}" CACHE INTERNAL "VMA include directory")

# --- platform defines ----------------------------------------------------

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
if(WAYLAND_HEADERS)
  list(APPEND _aperture_vk_defs VK_USE_PLATFORM_WAYLAND_KHR)
endif()

# --- aperture::lib::vulkan_headers --------------------------------------

add_library(aperture_vulkan_headers INTERFACE)
add_library(aperture::lib::vulkan_headers ALIAS aperture_vulkan_headers)
set_target_properties(aperture_vulkan_headers PROPERTIES EXPORT_NAME lib::vulkan_headers)
target_include_directories(aperture_vulkan_headers INTERFACE
    $<BUILD_INTERFACE:${APERTURE_VULKAN_SDK_INCLUDE_DIR}>
)
target_compile_definitions(aperture_vulkan_headers INTERFACE ${_aperture_vk_defs})

# --- aperture::lib::volk ------------------------------------------------

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
target_include_directories(aperture_volk
    PUBLIC
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

# --- aperture::lib::vma -------------------------------------------------

add_library(aperture_vma STATIC
    "${PROJECT_SOURCE_DIR}/third-party/vulkan/vma.cpp"
)
add_library(aperture::lib::vma ALIAS aperture_vma)
set_target_properties(aperture_vma PROPERTIES
    OUTPUT_NAME aperture_vma
    EXPORT_NAME lib::vma
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    FOLDER "third-party"
)
target_include_directories(aperture_vma
    PUBLIC
        $<BUILD_INTERFACE:${APERTURE_VMA_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(aperture_vma PUBLIC aperture::lib::volk)
target_compile_options(aperture_vma PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W0>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
)

message(STATUS "Vulkan: aperture::lib::{vulkan_headers,volk,vma} ready (SDK v${APERTURE_VULKAN_SDK_VERSION})")
