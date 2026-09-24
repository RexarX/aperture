aperture_add_c_module(
    NAME cpu_root
    FOLDER "bindings/c"
    CPP_TARGET aperture::cpu_root
    HEADERS
        include/aperture/ext/cpu_root.h
    SOURCES
        src/cpu_root.cpp
)
