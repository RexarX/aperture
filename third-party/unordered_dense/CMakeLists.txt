add_library(aperture_unordered_dense INTERFACE)
add_library(aperture::lib::unordered_dense ALIAS aperture_unordered_dense)
target_include_directories(aperture_unordered_dense INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
