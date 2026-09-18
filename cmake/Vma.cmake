# Provide `aperture::lib::vma`.
#
# Default resolution:
#   1. A compatible target already in the parent project
#   2. Vulkan SDK when APERTURE_USE_VULKAN_SDK_VMA is ON and usable
#   3. find_package(VulkanMemoryAllocator) with version >= APERTURE_VMA_VERSION
#   4. CPM fetch of GPUOpen VulkanMemoryAllocator
#
# APERTURE_USE_SYSTEM_VMA skips the SDK and requires step 3.
#
# Static VMA with dynamic Vulkan functions via aperture::lib::volk.

include_guard(GLOBAL)
include(CPM)
include(PackageResolve)
include(Volk)

function(_aperture_accept_vma_dir DIR VERSION OUT_VAR)
  set(${OUT_VAR} FALSE PARENT_SCOPE)
  if(NOT DIR)
    return()
  endif()
  _aperture_is_sdk_path("${DIR}" _from_sdk)
  if(_from_sdk AND NOT APERTURE_USE_VULKAN_SDK_VMA)
    return()
  endif()
  if(VERSION)
    _aperture_version_meets("${VERSION}" "${APERTURE_VMA_VERSION}" _ok)
    if(NOT _ok)
      message(STATUS
          "VMA: ${VERSION} at '${DIR}' is older than ${APERTURE_VMA_VERSION}"
      )
      return()
    endif()
  endif()
  set(${OUT_VAR} TRUE PARENT_SCOPE)
endfunction()

function(_aperture_try_find_vma)
  find_package(VulkanMemoryAllocator ${APERTURE_VMA_VERSION} CONFIG QUIET)
  set(_ver "${VulkanMemoryAllocator_VERSION}")
  foreach(_tgt IN ITEMS GPUOpen::VulkanMemoryAllocator VulkanMemoryAllocator)
    if(TARGET ${_tgt})
      _aperture_include_dir_from_target(${_tgt} "vk_mem_alloc.h" _dir)
      _aperture_accept_vma_dir("${_dir}" "${_ver}" _ok)
      if(_ok)
        set(_aperture_vma_dir "${_dir}" PARENT_SCOPE)
        set(_aperture_vma_source "package" PARENT_SCOPE)
        return()
      endif()
    endif()
  endforeach()
endfunction()

function(_aperture_fetch_vma)
  message(STATUS "VMA: fetching v${APERTURE_VMA_VERSION} via CPM")
  CPMAddPackage(
      NAME VulkanMemoryAllocator
      GITHUB_REPOSITORY GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
      GIT_TAG "v${APERTURE_VMA_VERSION}"
      VERSION "${APERTURE_VMA_VERSION}"
      GIT_SHALLOW TRUE
      DOWNLOAD_ONLY YES
  )
  if(VulkanMemoryAllocator_SOURCE_DIR AND
      EXISTS "${VulkanMemoryAllocator_SOURCE_DIR}/include/vk_mem_alloc.h")
    set(_aperture_vma_dir "${VulkanMemoryAllocator_SOURCE_DIR}/include" PARENT_SCOPE)
    set(_aperture_vma_source "fetch" PARENT_SCOPE)
  endif()
endfunction()

if(TARGET aperture::lib::vma)
  return()
endif()

set(_aperture_vma_dir "")
set(_aperture_vma_source "")

foreach(_tgt IN ITEMS GPUOpen::VulkanMemoryAllocator VulkanMemoryAllocator)
  if(NOT _aperture_vma_dir)
    _aperture_include_dir_from_target(${_tgt} "vk_mem_alloc.h" _aperture_vma_dir)
  endif()
endforeach()
if(_aperture_vma_dir)
  set(_aperture_vma_source "provided")
endif()

if(NOT _aperture_vma_dir AND APERTURE_USE_VULKAN_SDK_VMA AND APERTURE_VULKAN_SDK_USABLE)
  _aperture_find_header_dir(_aperture_vma_dir "vk_mem_alloc.h"
      "${APERTURE_VULKAN_SDK_INCLUDE_DIR}/vma"
      "${APERTURE_VULKAN_SDK_INCLUDE_DIR}"
  )
  if(_aperture_vma_dir)
    set(_aperture_vma_source "sdk")
  else()
    message(STATUS
        "VMA: SDK at '${APERTURE_VULKAN_SDK_DIR}' has no vk_mem_alloc.h; "
        "falling back"
    )
  endif()
endif()

if(NOT _aperture_vma_dir)
  _aperture_try_find_vma()
  if(NOT _aperture_vma_dir AND APERTURE_USE_SYSTEM_VMA)
    message(FATAL_ERROR
        "APERTURE_USE_SYSTEM_VMA is ON but find_package(VulkanMemoryAllocator) "
        "did not provide vk_mem_alloc.h ${APERTURE_VMA_VERSION}+."
    )
  endif()
endif()

if(NOT _aperture_vma_dir AND NOT APERTURE_USE_SYSTEM_VMA)
  _aperture_fetch_vma()
endif()

if(NOT _aperture_vma_dir)
  message(FATAL_ERROR
      "Could not locate vk_mem_alloc.h ${APERTURE_VMA_VERSION}+. Install a full "
      "Vulkan SDK, provide VMA via find_package, or allow CPM to fetch "
      "VulkanMemoryAllocator."
  )
endif()

set(APERTURE_VMA_INCLUDE_DIR "${_aperture_vma_dir}" CACHE INTERNAL "VMA include directory")

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
target_include_directories(aperture_vma PUBLIC
    $<BUILD_INTERFACE:${APERTURE_VMA_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(aperture_vma PUBLIC aperture::lib::volk)
target_compile_options(aperture_vma PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W0>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
)
if(APERTURE_BUILD_SHARED)
  set_target_properties(aperture_vma PROPERTIES
      CXX_VISIBILITY_PRESET hidden
      VISIBILITY_INLINES_HIDDEN ON
  )
endif()

message(STATUS
    "VMA: aperture::lib::vma ready (${_aperture_vma_source}) -> ${_aperture_vma_dir}"
)
