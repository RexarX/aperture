set(APERTURE_CORE_HEADERS
    include/aperture/utils/defer.hpp
    include/aperture/utils/format.hpp
    include/aperture/utils/macro.hpp
    include/aperture/adapter.hpp
    include/aperture/aperture.hpp
    include/aperture/assert.hpp
    include/aperture/capability.hpp
    include/aperture/commands.hpp
    include/aperture/compiler.hpp
    include/aperture/device.hpp
    include/aperture/format.hpp
    include/aperture/instance.hpp
    include/aperture/log.hpp
    include/aperture/math.hpp
    include/aperture/pipeline.hpp
    include/aperture/platform.hpp
    include/aperture/queue.hpp
    include/aperture/result.hpp
    include/aperture/surface.hpp
    include/aperture/swapchain.hpp
    include/aperture/sync.hpp
    include/aperture/texture.hpp
    include/aperture/types.hpp
)

set(APERTURE_CORE_SOURCES
    src/adapter.cpp
    src/assert.cpp
    src/device.cpp
    src/instance.cpp
    src/log.cpp
    src/queue.cpp
    src/pch.hpp
)

set(APERTURE_CORE_FILES
    ${APERTURE_CORE_HEADERS}
    ${APERTURE_CORE_SOURCES}
)

set(APERTURE_CORE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)

add_library(aperture_core STATIC ${APERTURE_CORE_FILES})
add_library(aperture::core ALIAS aperture_core)

set_target_properties(aperture_core PROPERTIES
    OUTPUT_NAME aperture_core
    EXPORT_NAME core
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

aperture_apply_conventions(aperture_core)
aperture_target_set_output_dirs(aperture_core)
aperture_target_add_pch(aperture_core "${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp")

target_compile_definitions(aperture_core PUBLIC
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:APERTURE_ENABLE_ASSERT>
    APERTURE_PROJECT_VERSION="${PROJECT_VERSION}"
)

target_include_directories(aperture_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(APERTURE_ENABLE_VULKAN)
  target_compile_definitions(aperture_core PRIVATE APERTURE_HAS_VULKAN)
  target_include_directories(aperture_core PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/../backends/vulkan/include"
  )
  target_link_libraries(aperture_core PRIVATE aperture::lib::vulkan_headers)
endif()
if(APERTURE_BUILD_SHARED)
  target_link_libraries(aperture_core PRIVATE aperture::lib::slang)
else()
  target_link_libraries(aperture_core PUBLIC $<BUILD_INTERFACE:aperture::lib::slang>)
endif()

if(APERTURE_ENABLE_INSTALL)
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h" PATTERN "*.inl"
  )
endif()
