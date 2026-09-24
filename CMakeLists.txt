cmake_minimum_required(VERSION 3.30)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "4.0")
  set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum CMake policy version" FORCE)
endif()

project(aperture
    VERSION 0.1.0
    DESCRIPTION "Thin render interface, following the No Graphics API philosophy"
    HOMEPAGE_URL "https://github.com/RexarX/aperture"
    LANGUAGES C CXX
)

include(CMakeDependentOption)
include(GNUInstallDirs)

# Architecture
if(NOT (WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Linux" OR APPLE))
  message(FATAL_ERROR
      "Unsupported architecture or platform \"${CMAKE_SYSTEM_NAME}\", \"${CMAKE_SYSTEM_PROCESSOR}\""
  )
elseif((CMAKE_GENERATOR_PLATFORM MATCHES "ARM64") OR
       (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64"))
  set(BIN_ARCHITECTURE "arm64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(BIN_ARCHITECTURE "x64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(BIN_ARCHITECTURE "x86")
else()
  message(FATAL_ERROR
      "Unsupported architecture or platform \"${CMAKE_SYSTEM_NAME}\", \"${CMAKE_SYSTEM_PROCESSOR}\""
  )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_path(XLIB_HEADERS NAMES "X11/Xlib.h")
  find_path(XCB_HEADERS NAMES "xcb/xcb.h")
  find_path(WAYLAND_HEADERS NAMES "wayland-client.h")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(PROJECT_IS_TOP_LEVEL)
  if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
    message(STATUS "Setting build type to 'Release' as none was specified.")
  endif()

  if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo;Release" CACHE STRING "" FORCE)
  endif()

  set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(PROJECT_IS_TOP_LEVEL)
  if(MSVC OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
      AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Ob2 /DNDEBUG /Zi"
        CACHE STRING "Flags used by the CXX compiler during RELWITHDEBINFO builds." FORCE)
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "/O2 /Ob2 /DNDEBUG /Zi"
        CACHE STRING "Flags used by the C compiler during RELWITHDEBINFO builds." FORCE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG"
        CACHE STRING "Flags used by the CXX compiler during RELWITHDEBINFO builds." FORCE)
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG"
        CACHE STRING "Flags used by the C compiler during RELWITHDEBINFO builds." FORCE)
  endif()
endif()

option(APERTURE_BUILD_C_BINDINGS "Build C bindings" ${PROJECT_IS_TOP_LEVEL})
option(APERTURE_BUILD_EXAMPLES "Build examples" ${PROJECT_IS_TOP_LEVEL})
option(APERTURE_BUILD_SHARED "Build aperture as a shared library" OFF)
if(APPLE)
  option(APERTURE_ENABLE_VULKAN "Compile the Vulkan backend" OFF)
else()
  option(APERTURE_ENABLE_VULKAN "Compile the Vulkan backend" ON)
endif()
option(APERTURE_ENABLE_VALIDATION_SUPPORT
    "Compile validation support (opt in at runtime via InstanceDesc::enable_validation)"
    ON
)
option(APERTURE_ENABLE_CPU_ROOT_EXTENSION "Enable cpu root extension" ON)
option(APERTURE_ENABLE_IMGUI_EXTENSION "Enable imgui extension" OFF)
option(APERTURE_DEVELOPER_MODE "Enable developer-focused build features (sanitizers)" OFF)
option(APERTURE_ENABLE_RELEASE_LTO "Enable Link Time Optimization for Release builds" ${PROJECT_IS_TOP_LEVEL})
option(APERTURE_ENABLE_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(APERTURE_ENABLE_INSTALL "Enable installation rules" OFF)

cmake_dependent_option(APERTURE_ENABLE_LTO_RELWITHDEBINFO "Enable LTO for RelWithDebInfo" OFF
                       "APERTURE_ENABLE_RELEASE_LTO" OFF)
option(APERTURE_MANAGE_TOOLCHAIN
    "Allow Aperture to set CMAKE_LINKER / CMAKE_AR / CMAKE_RANLIB for the whole project"
    ${PROJECT_IS_TOP_LEVEL})

set(APERTURE_MIN_VULKAN_SDK_VERSION "1.4.357" CACHE STRING
    "Minimum Vulkan SDK / Vulkan-Headers version"
)
set(APERTURE_VULKAN_HEADERS_VERSION "1.4.357" CACHE STRING
    "Vulkan-Headers version fetched via CPM when the SDK / find_package is unused"
)
set(APERTURE_VOLK_VERSION "1.4.357.0" CACHE STRING
    "volk version fetched via CPM when the SDK / find_package is unused"
)
set(APERTURE_VMA_VERSION "3.4.0" CACHE STRING
    "VulkanMemoryAllocator version fetched via CPM when the SDK / find_package is unused"
)

option(APERTURE_USE_VULKAN_SDK
    "Allow using packages from a located Vulkan SDK"
    ON
)
option(APERTURE_USE_SYSTEM_VULKAN_HEADERS
    "Require Vulkan headers from find_package (VulkanHeaders or Vulkan)"
    OFF
)
option(APERTURE_USE_SYSTEM_VOLK
    "Require volk from find_package"
    OFF
)
option(APERTURE_USE_SYSTEM_VMA
    "Require VulkanMemoryAllocator from find_package"
    OFF
)
option(APERTURE_USE_SYSTEM_GLFW
    "Require glfw3 from find_package"
    OFF
)

cmake_dependent_option(APERTURE_USE_VULKAN_SDK_VK_HEADERS
    "Use Vulkan headers from the Vulkan SDK"
    ON
    "APERTURE_USE_VULKAN_SDK;NOT APERTURE_USE_SYSTEM_VULKAN_HEADERS"
    OFF
)
cmake_dependent_option(APERTURE_USE_VULKAN_SDK_VOLK
    "Use volk from the Vulkan SDK"
    ON
    "APERTURE_USE_VULKAN_SDK;NOT APERTURE_USE_SYSTEM_VOLK"
    OFF
)
cmake_dependent_option(APERTURE_USE_VULKAN_SDK_VMA
    "Use VMA from the Vulkan SDK"
    ON
    "APERTURE_USE_VULKAN_SDK;NOT APERTURE_USE_SYSTEM_VMA"
    OFF
)

include(ClangArchiver)
aperture_configure_clang_archiver()
include(Linker)
aperture_configure_linker()

if(APERTURE_ENABLE_RELEASE_LTO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT APERTURE_IPO_SUPPORTED OUTPUT error)
  if(APERTURE_IPO_SUPPORTED)
    set(APERTURE_IPO_SUPPORTED "${APERTURE_IPO_SUPPORTED}" CACHE INTERNAL "IPO/LTO support result")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    if(APERTURE_ENABLE_LTO_RELWITHDEBINFO)
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)
      message(STATUS "Link-Time Optimization (LTO) enabled for Release and RelWithDebInfo builds")
    else()
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO OFF)
      message(STATUS "Link-Time Optimization (LTO) enabled for Release builds (RelWithDebInfo LTO off)")
    endif()
    aperture_configure_lto_mode()
  else()
    message(STATUS "IPO/LTO not supported: ${error}")
    set(APERTURE_ENABLE_RELEASE_LTO OFF)
  endif()
  set(APERTURE_IPO_CACHED TRUE CACHE INTERNAL "IPO/LTO support has been checked globally")
endif()

if(CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  message(STATUS "Position-Independent Code (PIC) enabled for multi-config generators")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  message(STATUS "Position-Independent Code (PIC) enabled on Linux")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" OR CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  message(STATUS "Position-Independent Code (PIC) enabled for ${CMAKE_BUILD_TYPE} builds")
endif()

include(Sanitizers)
include(TargetUtils)

aperture_apply_linker()
aperture_print_sanitizer_status()

set(APERTURE_ROOT_DIR ${PROJECT_SOURCE_DIR})

# Downloads live in <root>/.cpm_cache so every preset reuses the same sources.
# Per-preset FetchContent build trees stay under CMAKE_BINARY_DIR/_deps.
# CPM.cmake writes CPM_SOURCE_CACHE=OFF into the cache when unset; replace that
# default, but keep an explicit user or environment path.
if(DEFINED ENV{CPM_SOURCE_CACHE})
  set(CPM_SOURCE_CACHE "$ENV{CPM_SOURCE_CACHE}" CACHE PATH
      "Directory to download CPM dependencies" FORCE
  )
elseif(NOT CPM_SOURCE_CACHE OR CPM_SOURCE_CACHE STREQUAL "OFF")
  set(CPM_SOURCE_CACHE "${APERTURE_ROOT_DIR}/.cpm_cache" CACHE PATH
      "Directory to download CPM dependencies" FORCE
  )
endif()
if(FETCHCONTENT_BASE_DIR STREQUAL "${APERTURE_ROOT_DIR}/build/_deps")
  unset(FETCHCONTENT_BASE_DIR CACHE)
endif()

include(CPM)
message(STATUS "CPM source cache: ${CPM_SOURCE_CACHE}")

# Locate the Vulkan SDK when allowed. Individual packages (headers, volk,
# VMA) decide whether to use it via APERTURE_USE_VULKAN_SDK_*.
include(VulkanSDK)

if(APERTURE_ENABLE_VULKAN)
  include(VulkanHeaders)
  include(Volk)
  include(Vma)
endif()

add_subdirectory(third-party)
add_subdirectory(src)

if(APERTURE_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(APERTURE_ENABLE_INSTALL)
  include(Install)
endif()
