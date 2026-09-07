function(aperture_add_example)
  cmake_parse_arguments(
    ARG
    ""
    "NAME"
    "SOURCES;HEADERS"
    ${ARGN}
  )

  if(NOT ARG_NAME)
    message(FATAL_ERROR "aperture_add_example: NAME is required")
  endif()

  if(NOT ARG_SOURCES)
    message(FATAL_ERROR "aperture_add_example: SOURCES is required")
  endif()

  set(_target "${ARG_NAME}_example")

  add_executable(${_target} ${ARG_HEADERS} ${ARG_SOURCES})

  aperture_apply_conventions(${_target})
  aperture_target_set_output_dirs(${_target} CUSTOM_FOLDER examples)
  aperture_target_set_folder(${_target} "examples")

  target_link_libraries(${_target} PRIVATE aperture::aperture)
  aperture_copy_slang_runtime(${_target})
endfunction()

aperture_add_example(
    NAME basic
    SOURCES basic.cpp
)
