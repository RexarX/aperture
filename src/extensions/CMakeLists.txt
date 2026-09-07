function(aperture_add_extension)
  cmake_parse_arguments(ARG "" "NAME" "SOURCES;HEADERS;PRIVATE_HEADERS" ${ARGN})

  if(NOT ARG_NAME)
    message(FATAL_ERROR "aperture_add_extension: NAME is required")
  endif()

  set(_target "aperture_${ARG_NAME}")
  set(_files ${ARG_HEADERS} ${ARG_SOURCES} ${ARG_PRIVATE_HEADERS})

  if(NOT _files)
    message(FATAL_ERROR "aperture_add_extension(${ARG_NAME}): SOURCES/HEADERS are required")
  endif()

  add_library(${_target} STATIC ${_files})
  add_library(aperture::${ARG_NAME} ALIAS ${_target})

  set_target_properties(${_target} PROPERTIES
      OUTPUT_NAME ${_target}
      EXPORT_NAME ${ARG_NAME}
  )

  aperture_apply_conventions(${_target})
  aperture_target_set_output_dirs(${_target})
  aperture_target_set_folder(${_target} "extensions")

  target_include_directories(${_target}
      PUBLIC
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      PRIVATE
          ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

  target_link_libraries(${_target} PUBLIC aperture::aperture)

  if(APERTURE_ENABLE_INSTALL)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h" PATTERN "*.inl" PATTERN "*.slang"
    )
  endif()

  set_property(GLOBAL APPEND PROPERTY APERTURE_EXTENSION_TARGETS ${_target})
endfunction()

if(APERTURE_ENABLE_CPU_ROOT_EXTENSION)
  add_subdirectory(cpu_root)
endif()

if(APERTURE_ENABLE_IMGUI_EXTENSION)
  add_subdirectory(imgui)
endif()
