set(APERTURE_CORE_HEADERS
    include/aperture/utils/format.hpp
    include/aperture/utils/macro.hpp
    include/aperture/aperture.hpp
    include/aperture/assert.hpp
    include/aperture/compiler.hpp
    include/aperture/log.hpp
    include/aperture/platform.hpp
)

set(APERTURE_CORE_SOURCES
    src/assert.cpp
    src/log.cpp
    src/pch.hpp
)

set(APERTURE_CORE_FILES
    ${APERTURE_CORE_HEADERS}
    ${APERTURE_CORE_SOURCES}
)

if(APERTURE_BUILD_SHARED)
  add_library(aperture SHARED ${APERTURE_CORE_FILES})
  aperture_target_set_shared_lib_platform(aperture)
  target_compile_definitions(aperture
      PUBLIC APERTURE_BUILD_SHARED
      PRIVATE APERTURE_EXPORTS
  )
else()
  add_library(aperture STATIC ${APERTURE_CORE_FILES})
endif()
add_library(aperture::aperture ALIAS aperture)

set_target_properties(aperture PROPERTIES
    EXPORT_NAME aperture
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

aperture_apply_conventions(aperture)
aperture_target_set_output_dirs(aperture)
aperture_target_add_pch(aperture "${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp")

target_compile_definitions(aperture PUBLIC
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:APERTURE_ENABLE_ASSERT>
    $<$<BOOL:APERTURE_ENABLE_VULKAN>:APERTURE_ENABLE_VULKAN>
    APERTURE_PROJECT_VERSION="${PROJECT_VERSION}"
)

target_include_directories(aperture
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(APERTURE_BUILD_SHARED)
  target_link_libraries(aperture PRIVATE aperture::lib::slang)
else()
  # Static .lib does not pull Slang in (MSVC). BUILD_INTERFACE PUBLIC so
  # in-tree consumers link it without putting Slang in the install export.
  target_link_libraries(aperture PUBLIC $<BUILD_INTERFACE:aperture::lib::slang>)
endif()

aperture_copy_slang_runtime(aperture)

if(APERTURE_ENABLE_INSTALL)
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h" PATTERN "*.inl"
  )
endif()
