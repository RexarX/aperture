if(APERTURE_ENABLE_CPU_ROOT_EXTENSION)
  add_subdirectory(cpu_root)
endif()

if(APERTURE_ENABLE_IMGUI_EXTENSION)
  add_subdirectory(imgui)
endif()
