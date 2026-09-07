aperture_add_backend(
    NAME vulkan

    PCH src/pch.hpp

    HEADERS
        include/aperture/vulkan.hpp
        src/VkBootstrap/VkBootstrap.h
        src/VkBootstrap/VkBootstrapDispatch.h
        src/VkBootstrap/VkBootstrapFeatureChain.h
        src/VkBootstrap/VkBootstrapFeatureChain.inl

    SOURCES
        src/VkBootstrap/VkBootstrap.cpp

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
if(WAYLAND_HEADERS)
  target_compile_definitions(aperture_vulkan PUBLIC APERTURE_ENABLE_WAYLAND)
endif()
