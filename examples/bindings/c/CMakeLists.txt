add_library(aperture_examples_c_common STATIC
    common/window.h
    common/window.c
)
add_library(aperture::examples::c::common ALIAS aperture_examples_c_common)

aperture_apply_conventions(aperture_examples_c_common)
aperture_target_set_output_dirs(aperture_examples_c_common CUSTOM_FOLDER examples)
aperture_target_set_folder(aperture_examples_c_common "examples/bindings/c")

target_include_directories(aperture_examples_c_common PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_include_directories(aperture_examples_c_common PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/common"
)
target_compile_definitions(aperture_examples_c_common PUBLIC GLFW_INCLUDE_NONE)
target_link_libraries(aperture_examples_c_common PUBLIC
    aperture::aperture
    glfw
)

if(XLIB_HEADERS)
  target_compile_definitions(aperture_examples_c_common PRIVATE APERTURE_EXAMPLES_X11)
endif()
if(WAYLAND_HEADERS)
  target_compile_definitions(aperture_examples_c_common PRIVATE APERTURE_EXAMPLES_WAYLAND)
endif()

add_executable(basic_c_example basic.c)
aperture_apply_conventions(basic_c_example)
aperture_target_set_output_dirs(basic_c_example CUSTOM_FOLDER examples)
aperture_target_set_folder(basic_c_example "examples/bindings/c")
target_link_libraries(basic_c_example PRIVATE aperture::examples::c::common)
if(APERTURE_ENABLE_VULKAN)
  target_link_libraries(basic_c_example PRIVATE aperture::c::vulkan)
endif()
