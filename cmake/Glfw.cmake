# Provide the `glfw` target for examples.
#
# Default resolution:
#   1. A glfw target already in the parent project
#   2. find_package(glfw3) with version >= APERTURE_GLFW_VERSION
#   3. CPM fetch from GitHub
#
# APERTURE_USE_SYSTEM_GLFW requires step 2.
# CPM sources are reused from CPM_SOURCE_CACHE (<root>/.cpm_cache by default).

include_guard(GLOBAL)
include(CPM)
include(PackageResolve)

set(APERTURE_GLFW_VERSION "3.5.1" CACHE STRING
    "GLFW version fetched from GitHub for examples if not found via find_package"
)

function(aperture_setup_glfw)
  if(TARGET glfw)
    return()
  endif()

  if(APERTURE_USE_SYSTEM_GLFW)
    find_package(glfw3 ${APERTURE_GLFW_VERSION} CONFIG REQUIRED)
  else()
    find_package(glfw3 ${APERTURE_GLFW_VERSION} CONFIG QUIET)
  endif()

  if(TARGET glfw)
    set(_ok TRUE)
    if(glfw3_VERSION)
      _aperture_version_meets("${glfw3_VERSION}" "${APERTURE_GLFW_VERSION}" _ok)
    endif()
    if(_ok)
      set_property(TARGET glfw PROPERTY IMPORTED_GLOBAL TRUE)
      if(DEFINED glfw3_VERSION)
        message(STATUS "GLFW: found ${glfw3_VERSION} via find_package")
      else()
        message(STATUS "GLFW: found via find_package")
      endif()
      return()
    endif()
    if(APERTURE_USE_SYSTEM_GLFW)
      message(FATAL_ERROR
          "APERTURE_USE_SYSTEM_GLFW is ON but glfw3 ${glfw3_VERSION} is older "
          "than ${APERTURE_GLFW_VERSION}."
      )
    endif()
    message(FATAL_ERROR
        "GLFW: find_package created glfw ${glfw3_VERSION} which is not "
        "${APERTURE_GLFW_VERSION}+. Uninstall it, or set "
        "APERTURE_USE_SYSTEM_GLFW=ON to use it anyway."
    )
  endif()

  if(APERTURE_USE_SYSTEM_GLFW)
    message(FATAL_ERROR
        "APERTURE_USE_SYSTEM_GLFW is ON but find_package(glfw3) did not "
        "provide glfw ${APERTURE_GLFW_VERSION}+."
    )
  endif()

  message(STATUS "GLFW: fetching ${APERTURE_GLFW_VERSION} via CPM")
  CPMAddPackage(
      NAME glfw
      GITHUB_REPOSITORY glfw/glfw
      GIT_TAG "${APERTURE_GLFW_VERSION}"
      VERSION "${APERTURE_GLFW_VERSION}"
      GIT_SHALLOW TRUE
      EXCLUDE_FROM_ALL YES
      OPTIONS
        "GLFW_BUILD_EXAMPLES OFF"
        "GLFW_BUILD_TESTS OFF"
        "GLFW_BUILD_DOCS OFF"
        "GLFW_INSTALL OFF"
        "GLFW_LIBRARY_TYPE STATIC"
  )

  if(TARGET glfw)
    set_target_properties(glfw PROPERTIES FOLDER "third-party")
  endif()
  if(TARGET update_mappings)
    set_target_properties(update_mappings PROPERTIES FOLDER "third-party")
  endif()
endfunction()
