aperture_add_c_module(
    NAME vulkan
    FOLDER "bindings/c"
    CPP_TARGET aperture::vulkan
    HEADERS
        include/aperture/vulkan/command/buffer.h
        include/aperture/vulkan/command/pool.h
        include/aperture/vulkan/memory/common.h
        include/aperture/vulkan/memory/free.h
        include/aperture/vulkan/memory/malloc.h
        include/aperture/vulkan/adapter.h
        include/aperture/vulkan/commands.h
        include/aperture/vulkan/device.h
        include/aperture/vulkan/format.h
        include/aperture/vulkan/header.h
        include/aperture/vulkan/instance.h
        include/aperture/vulkan/memory.h
        include/aperture/vulkan/queue.h
        include/aperture/vulkan/surface.h
        include/aperture/vulkan/swapchain.h
        include/aperture/vulkan/sync.h
        include/aperture/vulkan.h
    SOURCES
        src/command/buffer.cpp
        src/command/pool.cpp
        src/memory/common.cpp
        src/memory/free.cpp
        src/memory/malloc.cpp
        src/adapter.cpp
        src/device.cpp
        src/instance.cpp
        src/queue.cpp
        src/surface.cpp
        src/sync.cpp
)

target_include_directories(aperture_c_vulkan PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../../core/src"
)

target_link_libraries(aperture_c_vulkan PUBLIC aperture::lib::vulkan_headers)
target_compile_definitions(aperture_c_vulkan PUBLIC APERTURE_HAS_VULKAN)

if(XLIB_HEADERS)
  target_compile_definitions(aperture_c_vulkan PUBLIC APERTURE_ENABLE_XLIB)
endif()
if(XCB_HEADERS)
  target_compile_definitions(aperture_c_vulkan PUBLIC APERTURE_ENABLE_XCB)
endif()
if(WAYLAND_HEADERS)
  target_compile_definitions(aperture_c_vulkan PUBLIC APERTURE_ENABLE_WAYLAND)
endif()
