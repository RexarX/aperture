add_subdirectory(core)
add_subdirectory(backends)
add_subdirectory(extensions)
add_subdirectory(bindings)

set(_aperture_cxx_modules aperture_core)
get_property(_aperture_backend_targets GLOBAL PROPERTY APERTURE_BACKEND_TARGETS)
if(_aperture_backend_targets)
  list(APPEND _aperture_cxx_modules ${_aperture_backend_targets})
endif()
get_property(_aperture_ext_targets GLOBAL PROPERTY APERTURE_EXTENSION_TARGETS)
if(_aperture_ext_targets)
  list(APPEND _aperture_cxx_modules ${_aperture_ext_targets})
endif()

set(_aperture_modules ${_aperture_cxx_modules})
get_property(_aperture_c_targets GLOBAL PROPERTY APERTURE_C_TARGETS)
if(_aperture_c_targets)
  list(APPEND _aperture_modules ${_aperture_c_targets})
endif()

if(APERTURE_BUILD_SHARED)
  set(_aperture_glue_src "${CMAKE_CURRENT_BINARY_DIR}/aperture_glue.cpp")
  if(NOT EXISTS "${_aperture_glue_src}")
    file(WRITE "${_aperture_glue_src}" "/* aperture::aperture shared glue */\n")
  endif()

  add_library(aperture SHARED "${_aperture_glue_src}")
  aperture_target_set_shared_lib_platform(aperture)
  target_compile_definitions(aperture
      PUBLIC APERTURE_BUILD_SHARED
      PRIVATE APERTURE_EXPORTS
  )
  set_target_properties(aperture PROPERTIES
      OUTPUT_NAME aperture
      EXPORT_NAME aperture
      VERSION ${PROJECT_VERSION}
      SOVERSION ${PROJECT_VERSION_MAJOR}
      CXX_VISIBILITY_PRESET hidden
      VISIBILITY_INLINES_HIDDEN ON
  )
  aperture_apply_conventions(aperture)
  aperture_target_set_output_dirs(aperture)

  foreach(_mod IN LISTS _aperture_modules)
    target_compile_definitions(${_mod} PRIVATE
        APERTURE_BUILD_SHARED
        APERTURE_EXPORTS
    )
    set_target_properties(${_mod} PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        CXX_VISIBILITY_PRESET hidden
        C_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
    add_dependencies(aperture ${_mod})
    if(MSVC)
      target_link_options(aperture PRIVATE "/WHOLEARCHIVE:$<TARGET_FILE:${_mod}>")
    else()
      target_link_libraries(aperture PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,${_mod}>")
    endif()
  endforeach()

  aperture_glue_link_nonmodule_deps(aperture ${_aperture_modules})
else()
  add_library(aperture INTERFACE)
  set_target_properties(aperture PROPERTIES EXPORT_NAME aperture)

  foreach(_mod IN LISTS _aperture_modules)
    target_link_libraries(aperture INTERFACE "$<LINK_ONLY:${_mod}>")
  endforeach()
endif()

add_library(aperture::aperture ALIAS aperture)

if(APERTURE_BUILD_SHARED)
  set(_aperture_usage PUBLIC)
else()
  set(_aperture_usage INTERFACE)
endif()

# C++ usage requirements come from enabled C++ modules and their PUBLIC deps
# (Vulkan headers, volk, VMA, APERTURE_HAS_VULKAN, ...). LINK_ONLY / WHOLEARCHIVE
# would otherwise hide them. C consumers must not receive those paths.
set_property(GLOBAL PROPERTY APERTURE_GLUE_USAGE_WALKED_aperture_CXX "")
foreach(_mod IN LISTS _aperture_cxx_modules)
  aperture_glue_copy_usage_walk(aperture ${_aperture_usage} CXX ${_mod})
endforeach()

# C API headers (<aperture/*.h>, valid C++ via extern "C"). Promote without a
# language filter so both C and CXX TUs can include them. C++ headers stay
# CXX-gated above (.hpp vs .h under the same aperture/ prefix).
if(TARGET aperture_c)
  aperture_glue_copy_usage(aperture ${_aperture_usage} "" aperture_c)
endif()
