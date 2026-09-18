add_subdirectory(core)
add_subdirectory(backends)
add_subdirectory(extensions)
add_subdirectory(bindings)

set(_aperture_cxx_modules aperture_core)
if(TARGET aperture_vulkan)
  list(APPEND _aperture_cxx_modules aperture_vulkan)
endif()
get_property(_aperture_ext_targets GLOBAL PROPERTY APERTURE_EXTENSION_TARGETS)
if(_aperture_ext_targets)
  list(APPEND _aperture_cxx_modules ${_aperture_ext_targets})
endif()

set(_aperture_modules ${_aperture_cxx_modules})
if(TARGET aperture_c)
  list(APPEND _aperture_modules aperture_c)
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

  target_link_libraries(aperture PRIVATE aperture::lib::slang)
  if(TARGET aperture_vulkan)
    target_link_libraries(aperture PRIVATE
        aperture::lib::volk
        aperture::lib::vma
    )
  endif()

  aperture_copy_slang_runtime(aperture)
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

# C++ headers/defines apply to CXX consumers only. A C target that links
# aperture::aperture should see C bindings headers (when enabled), not C++.
target_include_directories(aperture ${_aperture_usage}
    $<$<COMPILE_LANGUAGE:CXX>:$<BUILD_INTERFACE:${APERTURE_CORE_INCLUDE_DIR}>>
    $<$<COMPILE_LANGUAGE:CXX>:$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>>
)
target_compile_definitions(aperture ${_aperture_usage}
    $<$<COMPILE_LANGUAGE:CXX>:$<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:APERTURE_ENABLE_ASSERT>>
    $<$<COMPILE_LANGUAGE:CXX>:APERTURE_PROJECT_VERSION="${PROJECT_VERSION}">
)

if(TARGET aperture_vulkan)
  target_include_directories(aperture ${_aperture_usage}
      $<$<COMPILE_LANGUAGE:CXX>:$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/backends/vulkan/include>>
  )
  target_compile_definitions(aperture ${_aperture_usage}
      $<$<COMPILE_LANGUAGE:CXX>:APERTURE_HAS_VULKAN>
  )
  if(APERTURE_ENABLE_VALIDATION_SUPPORT)
    target_compile_definitions(aperture ${_aperture_usage}
        $<$<COMPILE_LANGUAGE:CXX>:APERTURE_ENABLE_VALIDATION>
    )
  endif()
  get_target_property(_vk_hdr_incs aperture_vulkan_headers INTERFACE_INCLUDE_DIRECTORIES)
  if(_vk_hdr_incs)
    foreach(_inc IN LISTS _vk_hdr_incs)
      target_include_directories(aperture ${_aperture_usage}
          $<$<COMPILE_LANGUAGE:CXX>:${_inc}>
      )
    endforeach()
  endif()
  get_target_property(_vk_hdr_defs aperture_vulkan_headers INTERFACE_COMPILE_DEFINITIONS)
  if(_vk_hdr_defs)
    foreach(_def IN LISTS _vk_hdr_defs)
      target_compile_definitions(aperture ${_aperture_usage}
          $<$<COMPILE_LANGUAGE:CXX>:${_def}>
      )
    endforeach()
  endif()
  get_target_property(_volk_incs aperture_volk INTERFACE_INCLUDE_DIRECTORIES)
  if(_volk_incs)
    foreach(_inc IN LISTS _volk_incs)
      target_include_directories(aperture ${_aperture_usage}
          $<$<COMPILE_LANGUAGE:CXX>:${_inc}>
      )
    endforeach()
  endif()
  get_target_property(_vma_incs aperture_vma INTERFACE_INCLUDE_DIRECTORIES)
  if(_vma_incs)
    foreach(_inc IN LISTS _vma_incs)
      target_include_directories(aperture ${_aperture_usage}
          $<$<COMPILE_LANGUAGE:CXX>:${_inc}>
      )
    endforeach()
  endif()
endif()

if(APERTURE_ENABLE_CPU_ROOT_EXTENSION)
  target_include_directories(aperture ${_aperture_usage}
      $<$<COMPILE_LANGUAGE:CXX>:$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extensions/cpu_root/include>>
  )
endif()
if(APERTURE_ENABLE_IMGUI_EXTENSION)
  target_include_directories(aperture ${_aperture_usage}
      $<$<COMPILE_LANGUAGE:CXX>:$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extensions/imgui/include>>
  )
endif()

if(TARGET aperture_c)
  target_include_directories(aperture ${_aperture_usage}
      $<$<COMPILE_LANGUAGE:C>:$<BUILD_INTERFACE:${APERTURE_C_INCLUDE_DIR}>>
      $<$<COMPILE_LANGUAGE:C>:$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>>
  )
endif()
