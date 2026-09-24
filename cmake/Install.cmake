# Installation configuration for aperture
#
# Requires APERTURE_ENABLE_INSTALL=ON.
#
# `aperture::aperture` is the glue over every enabled module (core,
# backends, extensions, and C bindings when APERTURE_BUILD_C_BINDINGS).
# Shared builds produce one DLL; static builds keep separate archives and
# the glue is INTERFACE. C and C++ headers both install under
# include/aperture/ (*.h vs *.hpp). The glue itself does not use INCLUDES
# DESTINATION so C++-only third-party includes (Vulkan) stay
# COMPILE_LANGUAGE-gated. Consumers need a Vulkan SDK for
# headers only when this build used SDK headers.

include_guard(GLOBAL)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

install(TARGETS aperture
    EXPORT apertureTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(_aperture_export_with_includes)
if(TARGET aperture_vulkan_headers)
  list(APPEND _aperture_export_with_includes aperture_vulkan_headers)
endif()
if(NOT APERTURE_BUILD_SHARED)
  list(APPEND _aperture_export_with_includes aperture_core)
  if(TARGET aperture_volk)
    list(APPEND _aperture_export_with_includes aperture_volk)
  endif()
  if(TARGET aperture_vma)
    list(APPEND _aperture_export_with_includes aperture_vma)
  endif()
  get_property(_aperture_backend_targets GLOBAL PROPERTY APERTURE_BACKEND_TARGETS)
  if(_aperture_backend_targets)
    list(APPEND _aperture_export_with_includes ${_aperture_backend_targets})
  endif()
  get_property(_aperture_extension_targets GLOBAL PROPERTY APERTURE_EXTENSION_TARGETS)
  if(_aperture_extension_targets)
    list(APPEND _aperture_export_with_includes ${_aperture_extension_targets})
  endif()
endif()

if(_aperture_export_with_includes)
  install(TARGETS ${_aperture_export_with_includes}
      EXPORT apertureTargets
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
endif()

if(NOT APERTURE_BUILD_SHARED)
  get_property(_aperture_c_targets GLOBAL PROPERTY APERTURE_C_TARGETS)
  if(_aperture_c_targets)
    install(TARGETS ${_aperture_c_targets}
        EXPORT apertureTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
  endif()
endif()
if(TARGET aperture_c)
  install(TARGETS aperture_c
      EXPORT apertureTargets
  )
endif()

# --- volk / VMA headers (so installed Vulkan-backend consumers can compile)
if(APERTURE_VOLK_INCLUDE_DIR)
  file(TO_CMAKE_PATH "${APERTURE_VOLK_INCLUDE_DIR}" _aperture_volk_inc)
  if(EXISTS "${_aperture_volk_inc}/volk.h")
    install(FILES "${_aperture_volk_inc}/volk.h"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    if(EXISTS "${_aperture_volk_inc}/volk.c")
      install(FILES "${_aperture_volk_inc}/volk.c"
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      )
    endif()
  endif()
endif()
if(APERTURE_VMA_INCLUDE_DIR)
  file(TO_CMAKE_PATH "${APERTURE_VMA_INCLUDE_DIR}" _aperture_vma_inc)
  if(EXISTS "${_aperture_vma_inc}/vk_mem_alloc.h")
    install(FILES "${_aperture_vma_inc}/vk_mem_alloc.h"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
  endif()
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
