function(aperture_add_c_module)
  cmake_parse_arguments(ARG "" "NAME;FOLDER;CPP_TARGET" "SOURCES;HEADERS;LIBRARIES" ${ARGN})

  if(NOT ARG_NAME)
    message(FATAL_ERROR "aperture_add_c_module: NAME is required")
  endif()

  set(_target "aperture_c_${ARG_NAME}")
  set(_files ${ARG_HEADERS} ${ARG_SOURCES})

  if(NOT _files)
    message(FATAL_ERROR "aperture_add_c_module(${ARG_NAME}): SOURCES/HEADERS are required")
  endif()

  add_library(${_target} STATIC ${_files})
  add_library(aperture::c::${ARG_NAME} ALIAS ${_target})

  set_target_properties(${_target} PROPERTIES
      OUTPUT_NAME ${_target}
      EXPORT_NAME c::${ARG_NAME}
      C_STANDARD 11
      C_STANDARD_REQUIRED ON
      C_EXTENSIONS OFF
      CXX_VISIBILITY_PRESET hidden
      C_VISIBILITY_PRESET hidden
      VISIBILITY_INLINES_HIDDEN ON
      POSITION_INDEPENDENT_CODE ON
  )

  aperture_apply_conventions(${_target})
  aperture_target_set_output_dirs(${_target})
  if(ARG_FOLDER)
    aperture_target_set_folder(${_target} "${ARG_FOLDER}")
  else()
    aperture_target_set_folder(${_target} "bindings/c")
  endif()

  target_include_directories(${_target}
      PUBLIC
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      PRIVATE
          ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

  if(NOT ARG_NAME STREQUAL "core")
    target_link_libraries(${_target} PUBLIC aperture::c::core)
  endif()

  if(ARG_CPP_TARGET)
    target_link_libraries(${_target} PRIVATE ${ARG_CPP_TARGET})
  endif()

  if(ARG_LIBRARIES)
    target_link_libraries(${_target} PRIVATE ${ARG_LIBRARIES})
  endif()

  if(APERTURE_ENABLE_INSTALL)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
    )
  endif()

  set_property(GLOBAL APPEND PROPERTY APERTURE_C_TARGETS ${_target})
endfunction()

add_subdirectory(core)

if(APERTURE_ENABLE_VULKAN)
  add_subdirectory(backends)
endif()

add_subdirectory(extensions)

get_property(_aperture_c_targets GLOBAL PROPERTY APERTURE_C_TARGETS)

add_library(aperture_c INTERFACE)
add_library(aperture::c ALIAS aperture_c)
set_target_properties(aperture_c PROPERTIES EXPORT_NAME c)
aperture_target_set_folder(aperture_c "bindings/c")

foreach(_mod IN LISTS _aperture_c_targets)
  target_link_libraries(aperture_c INTERFACE "$<LINK_ONLY:${_mod}>")
  aperture_glue_copy_usage(aperture_c INTERFACE "" ${_mod})
endforeach()
