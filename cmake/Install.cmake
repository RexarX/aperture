# Installation configuration for aperture
#
# Requires APERTURE_ENABLE_INSTALL=ON.
#
# Core (aperture.lib) links Slang only. The Vulkan backend is a separate
# static library that publicly links volk and VMA. Consumers that want
# Vulkan link aperture::vulkan (which also pulls aperture::aperture).
# slang-compiler still ships beside bin/ because Slang is a shared library.
# Consumers still need a Vulkan SDK for headers when using the Vulkan backend.

include_guard(GLOBAL)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(_aperture_export_targets aperture aperture_vulkan_headers aperture_volk aperture_vma)
if(TARGET aperture_c)
  list(APPEND _aperture_export_targets aperture_c)
endif()
get_property(_aperture_backend_targets GLOBAL PROPERTY APERTURE_BACKEND_TARGETS)
if(_aperture_backend_targets)
  list(APPEND _aperture_export_targets ${_aperture_backend_targets})
endif()
get_property(_aperture_extension_targets GLOBAL PROPERTY APERTURE_EXTENSION_TARGETS)
if(_aperture_extension_targets)
  list(APPEND _aperture_export_targets ${_aperture_extension_targets})
endif()

install(TARGETS ${_aperture_export_targets}
    EXPORT apertureTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# --- Slang compiler runtime ----------------------------------------------
get_target_property(_aperture_slang_loc slang::slang IMPORTED_LOCATION_RELEASE)
if(NOT _aperture_slang_loc)
  get_target_property(_aperture_slang_loc slang::slang IMPORTED_LOCATION)
endif()
get_filename_component(APERTURE_SLANG_SONAME "${_aperture_slang_loc}" NAME)
file(TO_CMAKE_PATH "${_aperture_slang_loc}" _aperture_slang_loc)

set(APERTURE_SLANG_IMPLIB_NAME "")
if(WIN32)
  get_target_property(_aperture_slang_implib slang::slang IMPORTED_IMPLIB_RELEASE)
  if(NOT _aperture_slang_implib)
    get_target_property(_aperture_slang_implib slang::slang IMPORTED_IMPLIB)
  endif()
  if(_aperture_slang_implib)
    get_filename_component(APERTURE_SLANG_IMPLIB_NAME "${_aperture_slang_implib}" NAME)
    file(TO_CMAKE_PATH "${_aperture_slang_implib}" _aperture_slang_implib)
    install(FILES "${_aperture_slang_implib}" DESTINATION ${CMAKE_INSTALL_LIBDIR})
  endif()
endif()

if(WIN32)
  set(APERTURE_SLANG_RUNTIME_SUBDIR "${CMAKE_INSTALL_BINDIR}")
else()
  set(APERTURE_SLANG_RUNTIME_SUBDIR "${CMAKE_INSTALL_LIBDIR}")
endif()

install(FILES "${_aperture_slang_loc}" DESTINATION ${APERTURE_SLANG_RUNTIME_SUBDIR})

get_target_property(_aperture_slang_incs slang::slang INTERFACE_INCLUDE_DIRECTORIES)
if(_aperture_slang_incs)
  foreach(_inc IN LISTS _aperture_slang_incs)
    file(TO_CMAKE_PATH "${_inc}" _inc)
    if(EXISTS "${_inc}/slang.h")
      install(FILES "${_inc}/slang.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    endif()
    if(EXISTS "${_inc}/slang" AND IS_DIRECTORY "${_inc}/slang")
      install(DIRECTORY "${_inc}/slang/"
          DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/slang"
          FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
      )
    endif()
  endforeach()
endif()

# --- volk / VMA headers (so installed Vulkan-backend consumers can compile)
if(EXISTS "${APERTURE_VOLK_INCLUDE_DIR}/volk.h")
  install(FILES "${APERTURE_VOLK_INCLUDE_DIR}/volk.h"
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
  if(EXISTS "${APERTURE_VOLK_INCLUDE_DIR}/volk.c")
    install(FILES "${APERTURE_VOLK_INCLUDE_DIR}/volk.c"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
  endif()
endif()
if(EXISTS "${APERTURE_VMA_INCLUDE_DIR}/vk_mem_alloc.h")
  install(FILES "${APERTURE_VMA_INCLUDE_DIR}/vk_mem_alloc.h"
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
endif()

install(EXPORT apertureTargets
    FILE apertureTargets.cmake
    NAMESPACE aperture::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/aperture
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/apertureConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/apertureConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/apertureConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/aperture
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/apertureConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/apertureConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/aperture
)

message(STATUS "Install rules configured (install prefix: ${CMAKE_INSTALL_PREFIX})")
