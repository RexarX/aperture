aperture_add_c_module(
    NAME imgui
    FOLDER "bindings/c"
    CPP_TARGET aperture::imgui
    HEADERS
        include/aperture/ext/imgui.h
    SOURCES
        src/imgui.cpp
)
