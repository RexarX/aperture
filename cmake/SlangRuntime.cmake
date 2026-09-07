# Copy Slang's compiler shared library next to a consuming binary.
#
# Compile and reflection live in slang::slang. SPIR-V emission does not
# need DXC at runtime.

include_guard(GLOBAL)

function(aperture_copy_slang_runtime TARGET)
  if(NOT TARGET ${TARGET})
    message(FATAL_ERROR "aperture_copy_slang_runtime: \"${TARGET}\" is not a target")
  endif()

  get_target_property(_type ${TARGET} TYPE)
  if(NOT (_type STREQUAL "EXECUTABLE" OR _type STREQUAL "SHARED_LIBRARY" OR _type STREQUAL "MODULE_LIBRARY"))
    return()
  endif()

  if(NOT TARGET slang::slang)
    message(WARNING "aperture_copy_slang_runtime: slang::slang is not available; call aperture_setup_slang() first")
    return()
  endif()

  if(APPLE)
    set_property(TARGET ${TARGET} APPEND PROPERTY BUILD_RPATH "@loader_path")
  elseif(UNIX)
    set_property(TARGET ${TARGET} APPEND PROPERTY BUILD_RPATH "$ORIGIN")
  endif()

  add_custom_command(TARGET ${TARGET} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "$<TARGET_FILE:slang::slang>"
          "$<TARGET_FILE_DIR:${TARGET}>/"
      COMMENT "Copy slang compiler next to ${TARGET}"
      VERBATIM
  )
endfunction()
