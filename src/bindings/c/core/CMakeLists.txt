set(APERTURE_C_CORE_HEADERS
    include/aperture/command/buffer.h
    include/aperture/command/pool.h
    include/aperture/memory/bump_allocator.h
    include/aperture/memory/common.h
    include/aperture/memory/free.h
    include/aperture/memory/malloc.h
    include/aperture/memory/offset_allocator.h
    include/aperture/adapter.h
    include/aperture/aperture.h
    include/aperture/assert.h
    include/aperture/capability.h
    include/aperture/commands.h
    include/aperture/compiler.h
    include/aperture/device.h
    include/aperture/format.h
    include/aperture/instance.h
    include/aperture/log.h
    include/aperture/math.h
    include/aperture/memory.h
    include/aperture/pipeline.h
    include/aperture/platform.h
    include/aperture/queue.h
    include/aperture/result.h
    include/aperture/surface.h
    include/aperture/swapchain.h
    include/aperture/sync.h
    include/aperture/texture.h
    include/aperture/types.h
    include/aperture/version.h
)

set(APERTURE_C_CORE_SOURCES
    src/command/buffer.cpp
    src/command/pool.cpp
    src/memory/bump_allocator.cpp
    src/memory/common.cpp
    src/memory/free.cpp
    src/memory/malloc.cpp
    src/memory/offset_allocator.cpp
    src/adapter.cpp
    src/assert.cpp
    src/convert.hpp
    src/device.cpp
    src/instance.cpp
    src/log.cpp
    src/queue.cpp
    src/sync.cpp
    src/version.cpp
)

aperture_add_c_module(
    NAME core
    FOLDER "bindings/c"
    CPP_TARGET aperture::core
    HEADERS ${APERTURE_C_CORE_HEADERS}
    SOURCES ${APERTURE_C_CORE_SOURCES}
)

target_include_directories(aperture_c_core
    PUBLIC
        $<BUILD_INTERFACE:${APERTURE_CORE_GENERATED_INCLUDE_DIR}>
    PRIVATE
        ${APERTURE_CORE_INCLUDE_DIR}
)

target_compile_definitions(aperture_c_core PUBLIC
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:APERTURE_ENABLE_ASSERT>
)
if(APERTURE_ENABLE_VALIDATION_SUPPORT)
  target_compile_definitions(aperture_c_core PUBLIC APERTURE_ENABLE_VALIDATION_SUPPORT)
endif()
