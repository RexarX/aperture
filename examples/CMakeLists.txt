include(Glfw)
aperture_setup_glfw()

add_library(aperture_examples_common STATIC
    common/window.hpp
    common/window.cpp
)
add_library(aperture::examples::common ALIAS aperture_examples_common)

aperture_apply_conventions(aperture_examples_common)
aperture_target_set_output_dirs(aperture_examples_common CUSTOM_FOLDER examples)
aperture_target_set_folder(aperture_examples_common "examples")

target_include_directories(aperture_examples_common PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_compile_definitions(aperture_examples_common PUBLIC GLFW_INCLUDE_NONE)
target_link_libraries(aperture_examples_common PUBLIC
    aperture::aperture
    glfw
)

if(XLIB_HEADERS)
  target_compile_definitions(aperture_examples_common PRIVATE APERTURE_EXAMPLES_X11)
endif()
if(WAYLAND_HEADERS)
  target_compile_definitions(aperture_examples_common PRIVATE APERTURE_EXAMPLES_WAYLAND)
endif()

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

  target_link_libraries(${_target} PRIVATE aperture::examples::common)
  aperture_copy_slang_runtime(${_target})
endfunction()

aperture_add_example(
    NAME basic
    SOURCES basic.cpp
)
