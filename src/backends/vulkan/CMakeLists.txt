aperture_add_backend(
    NAME vulkan

    HEADERS
        include/aperture/vulkan/adapter.hpp
        include/aperture/vulkan/device.hpp
        include/aperture/vulkan/format.hpp
        include/aperture/vulkan/header.hpp
        include/aperture/vulkan/instance.hpp
        include/aperture/vulkan/queue.hpp
        include/aperture/vulkan/surface.hpp
        include/aperture/vulkan/swapchain.hpp
        include/aperture/vulkan/x11_undef.hpp
        include/aperture/vulkan.hpp
        src/internal.hpp

    SOURCES
        src/adapter.cpp
        src/device.cpp
        src/instance.cpp
        src/queue.cpp
        src/surface.cpp

    PCH src/pch.hpp

    LIBRARIES
        aperture::lib::volk
        aperture::lib::vma
)

if(APERTURE_ENABLE_VALIDATION_SUPPORT)
  target_compile_definitions(aperture_vulkan PUBLIC APERTURE_ENABLE_VALIDATION)
endif()
if(XLIB_HEADERS)
  target_compile_definitions(aperture_vulkan PUBLIC APERTURE_ENABLE_XLIB)
endif()
if(XCB_HEADERS)
  target_compile_definitions(aperture_vulkan PUBLIC APERTURE_ENABLE_XCB)
endif()
if(WAYLAND_HEADERS)
  target_compile_definitions(aperture_vulkan PUBLIC APERTURE_ENABLE_WAYLAND)
endif()

target_compile_definitions(aperture_vulkan PUBLIC APERTURE_HAS_VULKAN)
