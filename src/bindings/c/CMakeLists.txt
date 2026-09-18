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

set(APERTURE_C_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)

add_library(aperture_c STATIC ${APERTURE_C_BINDINGS_FILES})
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
    POSITION_INDEPENDENT_CODE ON
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

target_link_libraries(aperture_c PUBLIC aperture::core)

if(APERTURE_ENABLE_INSTALL)
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      FILES_MATCHING PATTERN "*.h"
  )
endif()
