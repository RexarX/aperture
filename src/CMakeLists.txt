function(aperture_add_backend)
  cmake_parse_arguments(ARG "" "NAME;PCH" "SOURCES;HEADERS;LIBRARIES" ${ARGN})

  if(NOT ARG_NAME)
    message(FATAL_ERROR "aperture_add_backend: NAME is required")
  endif()

  set(_target "aperture_${ARG_NAME}")
  set(_files ${ARG_HEADERS} ${ARG_SOURCES} ${ARG_PCH})

  if(NOT _files)
    message(FATAL_ERROR "aperture_add_backend(${ARG_NAME}): SOURCES/HEADERS are required")
  endif()

  add_library(${_target} STATIC ${_files})
  add_library(aperture::${ARG_NAME} ALIAS ${_target})

  set_target_properties(${_target} PROPERTIES
      OUTPUT_NAME ${_target}
      EXPORT_NAME ${ARG_NAME}
      POSITION_INDEPENDENT_CODE ON
  )

  aperture_apply_conventions(${_target})
  aperture_target_set_output_dirs(${_target})
  aperture_target_set_folder(${_target} "backends")

  if(ARG_PCH)
    if(NOT IS_ABSOLUTE "${ARG_PCH}")
      set(ARG_PCH "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_PCH}")
    endif()
    aperture_target_add_pch(${_target} "${ARG_PCH}")
  endif()

  target_include_directories(${_target}
      PUBLIC
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      PRIVATE
          ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

  target_link_libraries(${_target} PUBLIC aperture::aperture ${ARG_LIBRARIES})

  if(APERTURE_ENABLE_INSTALL)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h" PATTERN "*.inl"
    )
  endif()

  set_property(GLOBAL APPEND PROPERTY APERTURE_BACKEND_TARGETS ${_target})
endfunction()

add_subdirectory(core)

if(APERTURE_ENABLE_VULKAN)
  add_subdirectory(vulkan)
endif()

add_subdirectory(extensions)
add_subdirectory(bindings)
