set(APERTURE_C_BINDINGS_HEADERS
    include/aperture/aperture.h
)

set(APERTURE_C_BINDINGS_SOURCES
    src/aperture.cpp
)

set(APERTURE_C_BINDINGS_FILES
    ${APERTURE_C_BINDINGS_HEADERS}
    ${APERTURE_C_BINDINGS_SOURCES}
)

if(APERTURE_BUILD_SHARED)
  add_library(aperture_c SHARED ${APERTURE_C_BINDINGS_FILES})
  aperture_target_set_shared_lib_platform(aperture_c)
  target_compile_definitions(aperture_c
      PUBLIC APERTURE_C_SHARED
      PRIVATE APERTURE_C_EXPORTS
  )
else()
  add_library(aperture_c STATIC ${APERTURE_C_BINDINGS_FILES})
endif()

add_library(aperture::c ALIAS aperture_c)

set_target_properties(aperture_c PROPERTIES
    OUTPUT_NAME aperture_c
    EXPORT_NAME c
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
    CXX_VISIBILITY_PRESET hidden
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

aperture_apply_conventions(aperture_c)
aperture_target_set_output_dirs(aperture_c)
aperture_target_set_folder(aperture_c "bindings")

target_include_directories(aperture_c
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(aperture_c PRIVATE aperture::aperture)

if(APERTURE_BUILD_SHARED)
  aperture_copy_slang_runtime(aperture_c)
endif()

if(APERTURE_ENABLE_INSTALL)
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      FILES_MATCHING PATTERN "*.h"
  )
endif()
