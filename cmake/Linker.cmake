# Aperture fast-linker selection and RelWithDebInfo LTO mode
#
# Detects radlink/mold/lld/lld-link based on APERTURE_LINKER. Detection never
# mutates CMAKE_LINKER (that would break check_ipo_supported with RAD). Apply
# CMAKE_LINKER / -fuse-ld / LINKER_TYPE after the IPO probe via
# aperture_apply_linker().
#
# RelWithDebInfo ThinLTO / parallel WHOPR / MSVC incremental LTCG is applied
# only when APERTURE_ENABLE_LTO_RELWITHDEBINFO is ON, per-target by
# aperture_target_apply_lto_mode() (called from aperture_target_enable_lto).

include_guard(GLOBAL)

if(NOT DEFINED APERTURE_ENABLE_LTO_RELWITHDEBINFO)
  set(APERTURE_ENABLE_LTO_RELWITHDEBINFO OFF)
endif()

if(NOT DEFINED APERTURE_LINKER)
  if(PROJECT_IS_TOP_LEVEL)
    set(_aperture_linker_default "AUTO")
  else()
    set(_aperture_linker_default "DEFAULT")
  endif()
  set(APERTURE_LINKER "${_aperture_linker_default}" CACHE STRING
      "Linker to use: AUTO, MOLD, LLD, RAD, DEFAULT")
endif()
set_property(CACHE APERTURE_LINKER PROPERTY STRINGS "AUTO" "MOLD" "LLD" "RAD" "DEFAULT")

function(_aperture_is_msvc_abi OUT_VAR)
  if(MSVC OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
      AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
    set(${OUT_VAR} TRUE PARENT_SCOPE)
  else()
    set(${OUT_VAR} FALSE PARENT_SCOPE)
  endif()
endfunction()

function(_aperture_is_clang_cl OUT_VAR)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
      AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(${OUT_VAR} TRUE PARENT_SCOPE)
  else()
    set(${OUT_VAR} FALSE PARENT_SCOPE)
  endif()
endfunction()

function(_aperture_config_needs_lto OUT_VAR CONFIG)
  set(_needs FALSE)
  if(APERTURE_ENABLE_RELEASE_LTO AND APERTURE_IPO_SUPPORTED)
    if(CONFIG STREQUAL "Release")
      set(_needs TRUE)
    elseif(CONFIG STREQUAL "RelWithDebInfo" AND APERTURE_ENABLE_LTO_RELWITHDEBINFO)
      set(_needs TRUE)
    endif()
  endif()
  set(${OUT_VAR} "${_needs}" PARENT_SCOPE)
endfunction()

function(_aperture_msvc_asan_enabled OUT_VAR)
  set(_on FALSE)
  if(APERTURE_DEVELOPER_MODE)
    set(_san ON)
    if(DEFINED APERTURE_ENABLE_SANITIZERS)
      set(_san ${APERTURE_ENABLE_SANITIZERS})
    endif()
    set(_asan ON)
    if(DEFINED APERTURE_SANITIZER_ADDRESS)
      set(_asan ${APERTURE_SANITIZER_ADDRESS})
    endif()
    if(_san AND _asan)
      set(_on TRUE)
    endif()
  endif()
  set(${OUT_VAR} "${_on}" PARENT_SCOPE)
endfunction()

#[[
    _aperture_pick_msvc_config_linker(<out> <config>)

    Picks rad, lld-link, or default for one MSVC-ABI configuration.
]]
function(_aperture_pick_msvc_config_linker OUT_VAR CONFIG)
  _aperture_is_clang_cl(_clang_cl)
  _aperture_config_needs_lto(_needs_lto "${CONFIG}")
  _aperture_msvc_asan_enabled(_asan_on)

  set(_pick "default")

  # ASan compile/link flags are Debug-only, even when APERTURE_DEVELOPER_MODE is
  # on for RelWithDebInfo/Release. RAD cannot relocate MSVC ASan objects.
  if(CONFIG STREQUAL "Debug" AND _asan_on)
    if(APERTURE_LINKER MATCHES "^(RAD|LLD)$")
      message(WARNING
          "APERTURE_LINKER=${APERTURE_LINKER} cannot link MSVC AddressSanitizer objects; "
          "using the default MSVC linker for Debug")
    endif()
  elseif(_needs_lto)
    if(_clang_cl)
      if(APERTURE_LLD_LINK_EXECUTABLE AND APERTURE_LINKER MATCHES "^(AUTO|LLD|RAD)$")
        set(_pick "lld-link")
        if(APERTURE_LINKER STREQUAL "RAD")
          message(WARNING
              "APERTURE_LINKER=RAD is incompatible with Clang ThinLTO for ${CONFIG}; "
              "using lld-link")
        endif()
      elseif(APERTURE_LINKER MATCHES "^(LLD|RAD)$")
        message(WARNING
            "APERTURE_LINKER=${APERTURE_LINKER} cannot perform LTO for ${CONFIG}; "
            "using the default linker")
      endif()
    else()
      if(APERTURE_LINKER MATCHES "^(RAD|LLD)$")
        message(WARNING
            "APERTURE_LINKER=${APERTURE_LINKER} cannot consume MSVC LTCG for ${CONFIG}; "
            "using the default MSVC linker")
      endif()
    endif()
  else()
    # clang-cl: prefer lld-link. RAD treats some clang-cl C++ COMDATs
    # (e.g. std::bad_alloc) as multiply defined.
    if(_clang_cl AND APERTURE_LINKER MATCHES "^(AUTO|LLD)$" AND APERTURE_LLD_LINK_EXECUTABLE)
      set(_pick "lld-link")
    elseif((NOT _clang_cl OR APERTURE_LINKER STREQUAL "RAD")
        AND APERTURE_LINKER MATCHES "^(AUTO|RAD)$" AND APERTURE_RAD_LINK_EXECUTABLE)
      if(_clang_cl)
        message(WARNING
            "APERTURE_LINKER=RAD with clang-cl can fail on C++ COMDAT symbols; "
            "prefer APERTURE_LINKER=LLD")
      endif()
      set(_pick "rad")
    elseif(APERTURE_LINKER MATCHES "^(AUTO|LLD)$" AND APERTURE_LLD_LINK_EXECUTABLE)
      set(_pick "lld-link")
    elseif(APERTURE_LINKER STREQUAL "RAD" AND NOT APERTURE_RAD_LINK_EXECUTABLE)
      message(WARNING "APERTURE_LINKER=RAD requested but radlink was not found on PATH; "
          "falling back to the default MSVC linker")
    elseif(APERTURE_LINKER STREQUAL "LLD" AND NOT APERTURE_LLD_LINK_EXECUTABLE)
      message(WARNING "APERTURE_LINKER=LLD requested but lld-link was not found on PATH; "
          "falling back to the default MSVC linker")
    endif()
  endif()

  set(${OUT_VAR} "${_pick}" PARENT_SCOPE)
endfunction()

function(_aperture_linker_to_cmake_type OUT_VAR KIND)
  if(KIND STREQUAL "rad")
    set(${OUT_VAR} "rad" PARENT_SCOPE)
  elseif(KIND STREQUAL "lld-link" OR KIND STREQUAL "lld")
    set(${OUT_VAR} "LLD" PARENT_SCOPE)
  elseif(KIND STREQUAL "mold")
    set(${OUT_VAR} "MOLD" PARENT_SCOPE)
  else()
    set(${OUT_VAR} "" PARENT_SCOPE)
  endif()
endfunction()

# RAD Linker rejects CMake vs_link's /MANIFEST:EMBED,ID=N. /MANIFEST:NO makes
# vs_link skip that flag. RelWithDebInfo also ships /INCREMENTAL from CMake
# defaults plus Helios /INCREMENTAL:NO — drop the default when RAD is used.
function(_aperture_rad_compat_genex OUT_VAR DEBUG_KIND RELWITH_KIND RELEASE_KIND)
  set(_genex "")
  if(DEBUG_KIND STREQUAL "rad")
    string(APPEND _genex "$<$<CONFIG:Debug>:/MANIFEST:NO>")
  endif()
  if(RELWITH_KIND STREQUAL "rad")
    string(APPEND _genex "$<$<CONFIG:RelWithDebInfo>:/MANIFEST:NO>")
  endif()
  if(RELEASE_KIND STREQUAL "rad")
    string(APPEND _genex "$<$<CONFIG:Release>:/MANIFEST:NO>")
  endif()
  set(${OUT_VAR} "${_genex}" PARENT_SCOPE)
endfunction()

function(_aperture_strip_msvc_incremental_flag FLAG_VAR)
  if(NOT DEFINED ${FLAG_VAR})
    return()
  endif()
  set(_flags "${${FLAG_VAR}}")
  string(REGEX REPLACE "(^| )[/-]INCREMENTAL:YES" " " _flags "${_flags}")
  string(REGEX REPLACE "(^| )[/-]INCREMENTAL( |$)" " " _flags "${_flags}")
  string(STRIP "${_flags}" _flags)
  set(${FLAG_VAR} "${_flags}" CACHE STRING "" FORCE)
endfunction()

function(_aperture_append_msvc_link_flag FLAG_VAR FLAG)
  set(_flags "")
  if(DEFINED ${FLAG_VAR})
    set(_flags "${${FLAG_VAR}}")
  endif()
  string(FIND "${_flags}" "${FLAG}" _found)
  if(NOT _found EQUAL -1)
    return()
  endif()
  string(STRIP "${_flags} ${FLAG}" _flags)
  set(${FLAG_VAR} "${_flags}" CACHE STRING "" FORCE)
endfunction()

function(_aperture_apply_rad_msvc_link_flags KIND CONFIG_UPPER)
  if(NOT KIND STREQUAL "rad")
    return()
  endif()
  foreach(_prefix EXE SHARED MODULE)
    set(_var "CMAKE_${_prefix}_LINKER_FLAGS_${CONFIG_UPPER}")
    if(NOT CONFIG_UPPER STREQUAL "DEBUG")
      _aperture_strip_msvc_incremental_flag(${_var})
    endif()
    # vs_link only skips /MANIFEST:EMBED,ID=N when this token is on the
    # link command (add_link_options is not always visible to vs_link).
    _aperture_append_msvc_link_flag(${_var} "/MANIFEST:NO")
  endforeach()
endfunction()

#[[
    aperture_configure_linker()

    Finds radlink / lld-link / mold / ld.lld. Does not set CMAKE_LINKER.
    Call aperture_apply_linker() after check_ipo_supported().
]]
function(aperture_configure_linker)
  if(CMAKE_LINKER AND NOT APERTURE_SAVED_CMAKE_LINKER)
    set(APERTURE_SAVED_CMAKE_LINKER "${CMAKE_LINKER}" CACHE INTERNAL
        "CMAKE_LINKER before Helios fast-linker override")
  endif()
  set(_cache_key "${APERTURE_LINKER}")
  if(DEFINED APERTURE_LINKER_DETECT_CACHED AND APERTURE_LINKER_DETECT_CACHE_KEY STREQUAL "${_cache_key}")
    return()
  endif()

  _aperture_is_msvc_abi(_msvc_abi)

  if(_msvc_abi)
    if(APERTURE_LINKER MATCHES "^(AUTO|RAD)$")
      find_program(APERTURE_RAD_LINK_EXECUTABLE
          NAMES radlink radlink.exe
          HINTS
              "$ENV{RAD_ROOT}"
              "$ENV{RAD_ROOT}/bin"
      )
    endif()
    if(APERTURE_LINKER MATCHES "^(AUTO|LLD|RAD)$")
      find_program(APERTURE_LLD_LINK_EXECUTABLE
          NAMES lld-link lld-link.exe
          HINTS
              "$ENV{VCINSTALLDIR}/Tools/Llvm/x64/bin"
              "$ENV{VCINSTALLDIR}/Tools/Llvm/bin"
              "C:/Program Files/LLVM/bin"
              "C:/Program Files (x86)/LLVM/bin"
      )
    endif()
    if(APERTURE_LINKER STREQUAL "MOLD")
      message(WARNING "APERTURE_LINKER=MOLD is not supported on the MSVC ABI; "
          "falling back to the default MSVC linker")
    endif()
    if(APERTURE_LINKER STREQUAL "RAD" AND NOT WIN32)
      message(WARNING "APERTURE_LINKER=RAD is Windows-only")
    endif()
  else()
    if(APERTURE_LINKER STREQUAL "RAD")
      message(WARNING "APERTURE_LINKER=RAD is Windows-only; falling back to AUTO selection")
    endif()

    set(_allow_mold TRUE)
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
      set(_allow_mold FALSE)
    endif()

    if(_allow_mold AND APERTURE_LINKER MATCHES "^(AUTO|MOLD)$")
      find_program(APERTURE_MOLD_EXECUTABLE mold)
    endif()

    if(APERTURE_LINKER MATCHES "^(AUTO|LLD|RAD)$")
      find_program(APERTURE_LLD_EXECUTABLE ld.lld)
    endif()

    if(APERTURE_LINKER STREQUAL "MOLD")
      if(NOT _allow_mold)
        message(WARNING "APERTURE_LINKER=MOLD requested but mold cannot link Mach-O; "
            "falling back to the default linker")
      elseif(NOT APERTURE_MOLD_EXECUTABLE)
        message(WARNING "APERTURE_LINKER=MOLD requested but mold was not found on PATH; "
            "falling back to the default linker")
      endif()
    elseif(APERTURE_LINKER STREQUAL "LLD" AND NOT APERTURE_LLD_EXECUTABLE)
      message(WARNING "APERTURE_LINKER=LLD requested but ld.lld was not found on PATH; "
          "falling back to the default linker")
    endif()
  endif()

  set(APERTURE_LINKER_DETECT_CACHED TRUE CACHE INTERNAL "Linker tools have been searched" FORCE)
  set(APERTURE_LINKER_DETECT_CACHE_KEY "${_cache_key}" CACHE INTERNAL "Linker detect cache key" FORCE)
endfunction()

#[[
    aperture_apply_linker()

    Selects per-config linkers and applies them. Must run after IPO support
    has been recorded (APERTURE_IPO_SUPPORTED / APERTURE_ENABLE_RELEASE_LTO).
]]
function(aperture_apply_linker)
  if(NOT DEFINED APERTURE_IPO_SUPPORTED)
    set(APERTURE_IPO_SUPPORTED FALSE)
  endif()

  if(APERTURE_LINKER STREQUAL "DEFAULT")
    set(APERTURE_ACTIVE_LINKER "default" CACHE INTERNAL "Active linker (Unix / summary)" FORCE)
    set(APERTURE_LINKER_DEBUG "default" CACHE INTERNAL "MSVC-ABI Debug linker" FORCE)
    set(APERTURE_LINKER_RELWITHDEBINFO "default" CACHE INTERNAL "MSVC-ABI RelWithDebInfo linker" FORCE)
    set(APERTURE_LINKER_RELEASE "default" CACHE INTERNAL "MSVC-ABI Release linker" FORCE)
    set(APERTURE_LINKER_TYPE_GENEX "" CACHE INTERNAL "LINKER_TYPE generator expression" FORCE)
    set(APERTURE_RAD_COMPAT_GENEX "" CACHE INTERNAL "RAD vs_link compat genex" FORCE)
    set(APERTURE_LINKER_CACHED TRUE CACHE INTERNAL "Linker selection cached" FORCE)
    set(APERTURE_LINKER_CACHE_KEY "${_cache_key}" CACHE INTERNAL "Linker selection cache key" FORCE)
    message(STATUS "Linker: using the platform default (APERTURE_LINKER=DEFAULT)")
    return()
  endif()

  _aperture_is_msvc_abi(_msvc_abi)
  set(_chosen "default")
  set(_debug "default")
  set(_relwith "default")
  set(_release "default")
  set(_type_genex "")

  if(_msvc_abi)
    _aperture_pick_msvc_config_linker(_debug Debug)
    _aperture_pick_msvc_config_linker(_relwith RelWithDebInfo)
    _aperture_pick_msvc_config_linker(_release Release)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      set(_chosen "${_debug}")
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
      set(_chosen "${_relwith}")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
      set(_chosen "${_release}")
    else()
      set(_chosen "${_debug}")
    endif()

    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.29")
      if(APERTURE_RAD_LINK_EXECUTABLE)
        set(CMAKE_C_USING_LINKER_rad "${APERTURE_RAD_LINK_EXECUTABLE}" CACHE INTERNAL "RAD linker tool (C)")
        set(CMAKE_CXX_USING_LINKER_rad "${APERTURE_RAD_LINK_EXECUTABLE}" CACHE INTERNAL "RAD linker tool (CXX)")
      endif()
      _aperture_linker_to_cmake_type(_dbg_ty "${_debug}")
      _aperture_linker_to_cmake_type(_rel_ty "${_relwith}")
      _aperture_linker_to_cmake_type(_relz_ty "${_release}")
      set(_type_genex "")
      if(_dbg_ty)
        string(APPEND _type_genex "$<$<CONFIG:Debug>:${_dbg_ty}>")
      endif()
      if(_rel_ty)
        string(APPEND _type_genex "$<$<CONFIG:RelWithDebInfo>:${_rel_ty}>")
      endif()
      if(_relz_ty)
        string(APPEND _type_genex "$<$<CONFIG:Release>:${_relz_ty}>")
      endif()
    endif()

    set(_multi_config FALSE)
    if(CMAKE_CONFIGURATION_TYPES)
      set(_multi_config TRUE)
    endif()

    if(APERTURE_MANAGE_TOOLCHAIN)
      if(_multi_config)
        if(CMAKE_VERSION VERSION_LESS "3.29")
          message(WARNING
              "Per-config RAD/lld-link on the Visual Studio generator requires CMake 3.29+. "
              "Using the default MSVC linker for all configs (Ninja presets are unaffected).")
          set(_debug "default")
          set(_relwith "default")
          set(_release "default")
          set(_chosen "default")
          set(_type_genex "")
        else()
          message(STATUS
              "Linker: MSVC ABI Debug=${_debug}, RelWithDebInfo=${_relwith}, Release=${_release} "
              "(APERTURE_LINKER=${APERTURE_LINKER}, per-config LINKER_TYPE)")
        endif()
      elseif(NOT _chosen STREQUAL "default")
        if(_chosen STREQUAL "rad" AND APERTURE_RAD_LINK_EXECUTABLE)
          set(CMAKE_LINKER "${APERTURE_RAD_LINK_EXECUTABLE}" CACHE FILEPATH "Linker" FORCE)
        elseif(_chosen STREQUAL "lld-link" AND APERTURE_LLD_LINK_EXECUTABLE)
          set(CMAKE_LINKER "${APERTURE_LLD_LINK_EXECUTABLE}" CACHE FILEPATH "Linker" FORCE)
        endif()
        message(STATUS
            "Linker: using \"${_chosen}\" project-wide for ${CMAKE_BUILD_TYPE} "
            "(APERTURE_LINKER=${APERTURE_LINKER}, APERTURE_MANAGE_TOOLCHAIN=ON)")
      else()
        if(APERTURE_SAVED_CMAKE_LINKER)
          set(CMAKE_LINKER "${APERTURE_SAVED_CMAKE_LINKER}" CACHE FILEPATH "Linker" FORCE)
        endif()
        message(STATUS
            "Linker: MSVC ABI using the default linker for ${CMAKE_BUILD_TYPE} "
            "(APERTURE_LINKER=${APERTURE_LINKER})")
      endif()
    else()
      message(STATUS
          "Linker: MSVC ABI Debug=${_debug}, RelWithDebInfo=${_relwith}, Release=${_release} "
          "available for Aperture targets (APERTURE_MANAGE_TOOLCHAIN=OFF)")
    endif()

    _aperture_apply_rad_msvc_link_flags("${_debug}" DEBUG)
    _aperture_apply_rad_msvc_link_flags("${_relwith}" RELWITHDEBINFO)
    _aperture_apply_rad_msvc_link_flags("${_release}" RELEASE)
    _aperture_rad_compat_genex(_rad_compat "${_debug}" "${_relwith}" "${_release}")
    set(APERTURE_RAD_COMPAT_GENEX "${_rad_compat}" CACHE INTERNAL "RAD vs_link compat genex" FORCE)
  else()
    set(_allow_mold TRUE)
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
      set(_allow_mold FALSE)
    endif()

    if(_allow_mold AND APERTURE_LINKER MATCHES "^(AUTO|MOLD)$" AND APERTURE_MOLD_EXECUTABLE)
      set(_chosen "mold")
    elseif(APERTURE_LINKER MATCHES "^(AUTO|LLD|RAD)$" AND APERTURE_LLD_EXECUTABLE)
      set(_chosen "lld")
    endif()

    set(_debug "${_chosen}")
    set(_relwith "${_chosen}")
    set(_release "${_chosen}")

    if(APERTURE_MANAGE_TOOLCHAIN AND NOT _chosen STREQUAL "default")
      if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.29")
        if(_chosen STREQUAL "mold")
          set(CMAKE_LINKER_TYPE "MOLD")
        elseif(_chosen STREQUAL "lld")
          set(CMAKE_LINKER_TYPE "LLD")
        endif()
      endif()
      add_link_options(
          $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang,AppleClang>>:-fuse-ld=${_chosen}>
      )
      message(STATUS "Linker: using \"${_chosen}\" project-wide (APERTURE_LINKER=${APERTURE_LINKER}, APERTURE_MANAGE_TOOLCHAIN=ON)")
    elseif(NOT _chosen STREQUAL "default")
      message(STATUS "Linker: \"${_chosen}\" available for Aperture targets (APERTURE_LINKER=${APERTURE_LINKER}, APERTURE_MANAGE_TOOLCHAIN=OFF)")
    else()
      message(STATUS "Linker: no fast linker found, using the platform default (APERTURE_LINKER=${APERTURE_LINKER})")
    endif()
  endif()

  set(APERTURE_ACTIVE_LINKER "${_chosen}" CACHE INTERNAL "Active linker (Unix / current config)" FORCE)
  set(APERTURE_LINKER_DEBUG "${_debug}" CACHE INTERNAL "MSVC-ABI Debug linker" FORCE)
  set(APERTURE_LINKER_RELWITHDEBINFO "${_relwith}" CACHE INTERNAL "MSVC-ABI RelWithDebInfo linker" FORCE)
  set(APERTURE_LINKER_RELEASE "${_release}" CACHE INTERNAL "MSVC-ABI Release linker" FORCE)
  set(APERTURE_LINKER_TYPE_GENEX "${_type_genex}" CACHE INTERNAL "LINKER_TYPE generator expression" FORCE)
  if(NOT _msvc_abi)
    set(APERTURE_RAD_COMPAT_GENEX "" CACHE INTERNAL "RAD vs_link compat genex" FORCE)
  endif()
  set(APERTURE_LINKER_CACHED TRUE CACHE INTERNAL "Linker selection cached" FORCE)
  set(APERTURE_LINKER_CACHE_KEY "${_cache_key}" CACHE INTERNAL "Linker selection cache key" FORCE)
endfunction()

#[[
    aperture_target_apply_linker(<target>)

    Applies the selected linker. On the MSVC ABI with CMake >= 3.29, always
    sets per-config LINKER_TYPE (needed even when APERTURE_MANAGE_TOOLCHAIN is ON).
    Unix project-wide -fuse-ld is applied in aperture_apply_linker() when managing
    the toolchain.
]]
function(aperture_target_apply_linker TARGET)
  if(NOT TARGET ${TARGET})
    return()
  endif()
  get_target_property(_type ${TARGET} TYPE)
  if(_type STREQUAL "INTERFACE_LIBRARY" OR _type STREQUAL "UTILITY")
    return()
  endif()

  _aperture_is_msvc_abi(_msvc_abi)

  if(_msvc_abi)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.29" AND APERTURE_LINKER_TYPE_GENEX)
      set_property(TARGET ${TARGET} PROPERTY LINKER_TYPE "${APERTURE_LINKER_TYPE_GENEX}")
    endif()
    if(APERTURE_RAD_COMPAT_GENEX AND NOT APERTURE_MANAGE_TOOLCHAIN)
      target_link_options(${TARGET} PRIVATE ${APERTURE_RAD_COMPAT_GENEX})
    endif()
    return()
  endif()

  if(APERTURE_MANAGE_TOOLCHAIN)
    return()
  endif()
  if(NOT APERTURE_ACTIVE_LINKER OR APERTURE_ACTIVE_LINKER STREQUAL "default")
    return()
  endif()

  if(APERTURE_ACTIVE_LINKER STREQUAL "mold" OR APERTURE_ACTIVE_LINKER STREQUAL "lld")
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.29")
      if(APERTURE_ACTIVE_LINKER STREQUAL "mold")
        set_property(TARGET ${TARGET} PROPERTY LINKER_TYPE MOLD)
      else()
        set_property(TARGET ${TARGET} PROPERTY LINKER_TYPE LLD)
      endif()
    endif()
    target_link_options(${TARGET} PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang,AppleClang>>:-fuse-ld=${APERTURE_ACTIVE_LINKER}>
    )
  endif()
endfunction()

#[[
    aperture_configure_lto_mode()

    Prepares RelWithDebInfo ThinLTO cache directory when RelWithDebInfo LTO is
    enabled. When APERTURE_MANAGE_TOOLCHAIN is ON, also applies ThinLTO / parallel
    LTO flags directory-wide. When embedded (manage off), only
    aperture_target_apply_lto_mode() is used.
]]
function(aperture_configure_lto_mode)
  if(NOT APERTURE_ENABLE_RELEASE_LTO OR NOT APERTURE_IPO_SUPPORTED)
    return()
  endif()

  if(DEFINED APERTURE_LTO_MODE_CACHED)
    return()
  endif()

  set(_lto_cache_dir "${CMAKE_BINARY_DIR}/lto-cache")
  file(MAKE_DIRECTORY "${_lto_cache_dir}")
  set(APERTURE_LTO_CACHE_DIR "${_lto_cache_dir}" CACHE INTERNAL "ThinLTO cache directory")

  if(APERTURE_ENABLE_LTO_RELWITHDEBINFO AND APERTURE_MANAGE_TOOLCHAIN)
    add_compile_options(
        $<$<AND:$<CXX_COMPILER_ID:Clang,AppleClang>,$<CONFIG:RelWithDebInfo>>:-flto=thin>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RelWithDebInfo>>:-flto=auto>
    )
    add_link_options(
        $<$<AND:$<CXX_COMPILER_ID:Clang,AppleClang>,$<CONFIG:RelWithDebInfo>>:-flto=thin>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RelWithDebInfo>>:-flto=auto>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/LTCG:INCREMENTAL>
    )
    if(WIN32)
      add_link_options(
          $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RelWithDebInfo>>:/lldltocache:${_lto_cache_dir}>
      )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
      add_link_options(
          $<$<AND:$<CXX_COMPILER_ID:Clang,AppleClang>,$<CONFIG:RelWithDebInfo>>:-Wl,-cache_path_lto,${_lto_cache_dir}>
      )
    else()
      add_link_options(
          $<$<AND:$<CXX_COMPILER_ID:Clang,AppleClang>,$<CONFIG:RelWithDebInfo>>:-Wl,--thinlto-cache-dir=${_lto_cache_dir}>
      )
    endif()
  endif()

  set(APERTURE_LTO_MODE_CACHED TRUE CACHE INTERNAL "RelWithDebInfo LTO mode configured")
  if(APERTURE_ENABLE_LTO_RELWITHDEBINFO)
    message(STATUS "LTO mode: RelWithDebInfo uses ThinLTO/parallel LTO (cache: ${_lto_cache_dir}); Release uses full LTO")
  else()
    message(STATUS "LTO mode: Release uses full LTO; RelWithDebInfo LTO is off (set APERTURE_ENABLE_LTO_RELWITHDEBINFO=ON to enable)")
  endif()
endfunction()

#[[
    aperture_target_apply_lto_mode(<target>)

    RelWithDebInfo: ThinLTO (Clang) / -flto=auto (GCC) / /LTCG:INCREMENTAL (MSVC)
    when APERTURE_ENABLE_LTO_RELWITHDEBINFO is ON.
    Release keeps CMake's full IPO flags from INTERPROCEDURAL_OPTIMIZATION.
]]
function(aperture_target_apply_lto_mode TARGET)
  if(NOT APERTURE_ENABLE_RELEASE_LTO OR NOT APERTURE_IPO_SUPPORTED OR NOT APERTURE_ENABLE_LTO_RELWITHDEBINFO)
    return()
  endif()
  if(APERTURE_MANAGE_TOOLCHAIN)
    return()
  endif()
  if(NOT TARGET ${TARGET})
    return()
  endif()
  get_target_property(_type ${TARGET} TYPE)
  if(_type STREQUAL "INTERFACE_LIBRARY" OR _type STREQUAL "UTILITY")
    return()
  endif()

  if(NOT APERTURE_LTO_CACHE_DIR)
    set(APERTURE_LTO_CACHE_DIR "${CMAKE_BINARY_DIR}/lto-cache")
    file(MAKE_DIRECTORY "${APERTURE_LTO_CACHE_DIR}")
  endif()

  target_compile_options(${TARGET} PRIVATE
      $<$<AND:$<CXX_COMPILER_ID:Clang,AppleClang>,$<CONFIG:RelWithDebInfo>>:-flto=thin>
      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RelWithDebInfo>>:-flto=auto>
  )
  target_link_options(${TARGET} PRIVATE
      $<$<AND:$<CXX_COMPILER_ID:Clang,AppleClang>,$<CONFIG:RelWithDebInfo>>:-flto=thin>
      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RelWithDebInfo>>:-flto=auto>
      $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/LTCG:INCREMENTAL>
  )

  if(WIN32)
    target_link_options(${TARGET} PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RelWithDebInfo>>:/lldltocache:${APERTURE_LTO_CACHE_DIR}>
    )
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_options(${TARGET} PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:Clang,AppleClang>,$<CONFIG:RelWithDebInfo>>:-Wl,-cache_path_lto,${APERTURE_LTO_CACHE_DIR}>
    )
  else()
    target_link_options(${TARGET} PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:Clang,AppleClang>,$<CONFIG:RelWithDebInfo>>:-Wl,--thinlto-cache-dir=${APERTURE_LTO_CACHE_DIR}>
    )
  endif()
endfunction()
