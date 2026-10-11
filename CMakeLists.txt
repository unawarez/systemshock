cmake_minimum_required(VERSION 3.28)

project(shockolate VERSION 0.7.8)

include(FeatureSummary)
include(FetchContent)

# C/C++ config:
# LANGUAGE_EXTENSIONS OFF is an attempt for non-MSVC devs to avoid writing
# things that will fail on MSVC.
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_LANGUAGE_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_LANGUAGE_EXTENSIONS OFF)

option(ENABLE_DEBUG_BLIT "Enable debugging blitter" OFF)
add_feature_info(ENABLE_DEBUG_BLIT ENABLE_DEBUG_BLIT "Enable debugging blitter")
option(ENABLE_OPENGL "Enable OpenGL support" ON)
add_feature_info(ENABLE_OPENGL ENABLE_OPENGL "Enable OpenGL support")

set(ENABLE_SDL2 "BUNDLED" CACHE STRING "Enable SDL2 (ON/BUNDLED, default BUNDLED)")
set_property(CACHE ENABLE_SDL2 PROPERTY STRINGS "ON" "BUNDLED")
add_feature_info(ENABLE_SDL2 ENABLE_SOUND "Enable SDL2 support")

set(ENABLE_SOUND "BUNDLED" CACHE STRING "Enable sound support (requires SDL2_mixer) (ON/BUNDLED/OFF, default BUNDLED)")
set_property(CACHE ENABLE_SOUND PROPERTY STRINGS "ON" "BUNDLED" "OFF")
add_feature_info(ENABLE_SOUND ENABLE_SOUND "Enable sound support (requires SDL2_mixer)")

set(ENABLE_FLUIDSYNTH "BUNDLED" CACHE STRING "Enable FluidSynth MIDI support (ON/BUNDLED/OFF, default BUNDLED)")
set_property(CACHE ENABLE_FLUIDSYNTH PROPERTY STRINGS "ON" "BUNDLED" "OFF")
add_feature_info(ENABLE_FLUIDSYNTH ENABLE_FLUIDSYNTH "Enable FluidSynth MIDI support")

# To keep a minimum dependency list, any created library should be static, even third-party dependencies.
set(BUILD_SHARED_LIBS FALSE CACHE BOOL "Force creation of static libraries, globally" FORCE)

# HAAAAX!!
add_definitions(-DSVGA_SUPPORT)

if(ENABLE_DEBUG_BLIT)
	add_definitions(-DDEBUGGING_BLIT)
endif()

add_compile_options(-fsigned-char -fno-strict-aliasing)

# Find OpenGL
if(ENABLE_OPENGL)
	find_package(OpenGL REQUIRED)
	add_definitions(-DUSE_OPENGL)
	if(WIN32)
		SET(GLEW_version 2.1.0)
		FetchContent_Declare(
			glew
			URL https://sourceforge.net/projects/glew/files/glew/${GLEW_version}/glew-${GLEW_version}.tgz
			SOURCE_SUBDIR build/cmake
			EXCLUDE_FROM_ALL
		)
		FetchContent_MakeAvailable(GLEW)

		message("GLEW src-dir '${glew_SOURCE_DIR}'")
		message("GLEW bin-dir '${glew_BINARY_DIR}'")

		list(APPEND OPENGL_INCLUDE_DIRS ${glew_SOURCE_DIR}/include)
		list(APPEND OPENGL_LIBRARIES glew_s winmm)
	endif(WIN32)
endif(ENABLE_OPENGL)

if(ENABLE_SDL2 MATCHES "ON")
	find_package(SDL2 REQUIRED)
	if(SDL2_FOUND)
		message(STATUS "SDL2 found: ${SDL2_INCLUDE_DIRS} ${SDL2_LIBRARIES}")
	endif(SDL2_FOUND)
endif(ENABLE_SDL2 MATCHES "ON")
if(ENABLE_SDL2 MATCHES "BUNDLED")
	set(SDL_VERSION 2.32.10)
	FetchContent_Declare(
		SDL2
		GIT_REPOSITORY https://github.com/libsdl-org/SDL
		GIT_TAG "release-${SDL_VERSION}"
		EXCLUDE_FROM_ALL
	)

	set(SDL_SHARED OFF CACHE BOOL "Disabled SDL shared lib" FORCE)
	set(SDL_TEST OFF CACHE BOOL "Disable SDL Test" FORCE)
	set(SDL_INSTALL OFF CACHE BOOL "Disable SDL installation" FORCE)

	FetchContent_MakeAvailable(SDL2)
	set(SDL2_INCLUDE_DIRS ${SDL2_SOURCE_DIR}/include)
	set(SDL2_LIBRARIES SDL2::SDL2main SDL2::SDL2-static)
endif(ENABLE_SDL2 MATCHES "BUNDLED")

if(ENABLE_SOUND MATCHES "ON")
	# FIXME applies only for *nix systems
    find_package(SDL2_mixer REQUIRED)
	add_definitions(-DUSE_SDL_MIXER=1)
endif(ENABLE_SOUND MATCHES "ON")
if(ENABLE_SOUND MATCHES "BUNDLED")
	SET(SDL_MIXER_VERSION 2.8.2)
	FetchContent_Declare(
		SDL2_mixer
		GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
		GIT_TAG release-${SDL_MIXER_VERSION}
		EXCLUDE_FROM_ALL
	)

	set(SDL2MIXER_OPUS OFF CACHE BOOL "Disable Opus" FORCE)
	set(SDL2MIXER_MOD OFF CACHE BOOL "Disable MOD" FORCE)
	set(SDL2MIXER_MP3 OFF CACHE BOOL "Disable MP3" FORCE)
	set(SDL2MIXER_MIDI_FLUIDSYNTH OFF CACHE BOOL "Disable FluidSynth" FORCE)
	set(SDL2MIXER_WAVPACK OFF CACHE BOOL "Disable WavPack" FORCE)
	set(SDL2MIXER_INSTALL OFF CACHE BOOL "Disable SDL_mixer installation" FORCE)

	FetchContent_MakeAvailable(SDL2_mixer)
	set(SDL2_MIXER_INCLUDE_DIRS ${SDL2_mixer_SOURCE_DIR}/include)
	set(SDL2_MIXER_LIBRARIES SDL2_mixer::SDL2_mixer-static)

	add_definitions(-DUSE_SDL_MIXER=1)
endif(ENABLE_SOUND MATCHES "BUNDLED")

if(ENABLE_FLUIDSYNTH MATCHES "ON")
	find_package(PkgConfig)
    pkg_check_modules(fluidsynth REQUIRED IMPORTED_TARGET fluidsynth)
	add_definitions("-DUSE_FLUIDSYNTH=1")
endif(ENABLE_FLUIDSYNTH MATCHES "ON")
if(ENABLE_FLUIDSYNTH MATCHES "BUNDLED")
	SET(FLUIDSYNTH_REVISION c539a8d9270ba5a3f7d6e460606483fc2ab1eb61)
	FetchContent_Declare(
		FluidSynth
		GIT_REPOSITORY https://github.com/EtherTyper/fluidsynth-lite.git
		GIT_TAG ${FLUIDSYNTH_REVISION}
		EXCLUDE_FROM_ALL
	)

	if ( WIN32 )
		# Sadly, FluidSynth-Lite has its CMakeLists.txt hardcoded to make its functions as exports.
		# Idea: Modify CMakeLists.txt as part of this block here?
		set(BUILD_SHARED_LIBS TRUE CACHE BOOL "Force shared libs for FluidSynth on Windows" FORCE)
	endif ( WIN32 )
	FetchContent_MakeAvailable(FluidSynth)
	message("FluidSynth src-dir '${FluidSynth_SOURCE_DIR}'")
	message("FluidSynth bin-dir '${FluidSynth_BINARY_DIR}'")
	if(APPLE)
		# This is a necessary hack; compiler errors for various things.
		target_compile_options(libfluidsynth PRIVATE -Wno-error=single-bit-bitfield-constant-conversion -Wno-unused-but-set-variable)
	endif()
	if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 15)
		# This is a necessary hack; gcc-15 warns (errors) if a static array is initialized
		# with a string that exceeds its storage. This is the case for fluid_defsfont.c,
		# where it initializes idlist.
		target_compile_options(libfluidsynth PRIVATE -Wno-error=unterminated-string-initialization)
	endif()
	set(FLUIDSYNTH_INCLUDE_DIRS ${FLUIDSYNTH_INCLUDE_DIR})

	set(BUILD_SHARED_LIBS FALSE CACHE BOOL "Restore creation of static libraries" FORCE)
	add_definitions("-DUSE_FLUIDSYNTH=1")
endif(ENABLE_FLUIDSYNTH MATCHES "BUNDLED")

include_directories(
	${SDL2_INCLUDE_DIRS}
	${SDL2_MIXER_INCLUDE_DIRS}
	${FLUIDSYNTH_INCLUDE_DIRS}
	${OPENGL_INCLUDE_DIRS}
)

if(NOT WIN32)
  # Find ALSA for Linux native MIDI
  # NOTE: this seems to require having 64-bit dev pacakges installed when building
  #  on 64-bit OS, even when building a 32-bit binary
  find_package(ALSA)
  if(ALSA_FOUND)
      message(STATUS "ALSA found")
      include_directories(${ALSA_INCLUDE_DIRS})
      add_definitions(-DUSE_ALSA=1)
  endif(ALSA_FOUND)
endif(NOT WIN32)

# Generate version based on project version
set(PROJECT_REVISION_STRING "")
find_package(Git)
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --always --dirty
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE git_describe_out
        ERROR_VARIABLE git_describe_error
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE git_describe_result
    )
    if(git_describe_result EQUAL 0)
        set(PROJECT_REVISION_STRING "-g${git_describe_out}")
    else()
        message(WARNING "git describe failed: ${git_describe_error}")
    endif()
endif(GIT_FOUND)

message(STATUS "Version is ${PROJECT_VERSION}${PROJECT_REVISION_STRING}")
configure_file("${CMAKE_SOURCE_DIR}/src/GameSrc/Headers/shockolate_version.h.in"
    "${CMAKE_BINARY_DIR}/generated/shockolate_version.h" )

# Configuration done. Print features
feature_summary(WHAT ENABLED_FEATURES DESCRIPTION "Enabled features:")
feature_summary(WHAT DISABLED_FEATURES DESCRIPTION "Disabled features:")

include(cmake/shaders.cmake)

# Sources configuration
add_subdirectory(src/Libraries/)

# MacSrc and GameSrc circularly depend on each other. Thus GAME_LIB was a lie.
add_executable(systemshock WIN32
	src/GameSrc/ai.c
	src/GameSrc/airupt.c
	src/GameSrc/amap.c
	src/GameSrc/amaploop.c
	src/GameSrc/ammomfd.c
	src/GameSrc/anim.c
	src/GameSrc/archiveformat.c
	src/GameSrc/audiolog.c
	src/GameSrc/automap.c
	src/GameSrc/bark.c
	src/GameSrc/biohelp.c
	src/GameSrc/cardmfd.c
	src/GameSrc/citres.c
	src/GameSrc/combat.c
	src/GameSrc/cone.c
	src/GameSrc/criterr.c
	src/GameSrc/cyber.c
	src/GameSrc/cybermfd.c
	src/GameSrc/cybmem.c
	src/GameSrc/cybrnd.c
	src/GameSrc/cutsloop.c
	src/GameSrc/damage.c
	src/GameSrc/digifx.c
	src/GameSrc/drugs.c
	src/GameSrc/effect.c
	src/GameSrc/email.c
	src/GameSrc/faceobj.c
	src/GameSrc/fixtrmfd.c
	src/GameSrc/frcamera.c
	src/GameSrc/frclip.c
	src/GameSrc/frcompil.c
	src/GameSrc/frmain.c
	src/GameSrc/frobj.c
	src/GameSrc/froslew.c
	src/GameSrc/frpipe.c
	src/GameSrc/frpts.c
	src/GameSrc/frsetup.c
	src/GameSrc/frtables.c
	src/GameSrc/frterr.c
	src/GameSrc/frutil.c
	src/GameSrc/FrUtils.c
	src/GameSrc/fullamap.c
	src/GameSrc/fullscrn.c
	src/GameSrc/gameloop.c
	src/GameSrc/gameobj.c
	src/GameSrc/gamesort.c
	src/GameSrc/gamestrn.c
	src/GameSrc/gamesys.c
	src/GameSrc/gametime.c
	src/GameSrc/gamewrap.c
	src/GameSrc/gearmfd.c
	src/GameSrc/gr2ss.c
	src/GameSrc/grenades.c
	src/GameSrc/hand.c
	src/GameSrc/hflip.c
	src/GameSrc/hkeyfunc.c
	src/GameSrc/hud.c
	src/GameSrc/hudobj.c
	src/GameSrc/init.c
	src/GameSrc/input.c
	src/GameSrc/invent.c
	src/GameSrc/leanmetr.c
	src/GameSrc/mainloop.c
	src/GameSrc/map.c
	src/GameSrc/mfdfunc.c
	src/GameSrc/mfdgadg.c
	src/GameSrc/mfdgames.c
	src/GameSrc/mfdgump.c
	src/GameSrc/mfdpanel.c
	src/GameSrc/minimax.c
	src/GameSrc/mlimbs.c
	src/GameSrc/movekeys.c
	src/GameSrc/musicai.c
	src/GameSrc/newai.c
	src/GameSrc/newmfd.c
	src/GameSrc/objapp.c
	src/GameSrc/objects.c
	src/GameSrc/objload.c
	src/GameSrc/objprop.c
	src/GameSrc/objsim.c
	src/GameSrc/objuse.c
	src/GameSrc/olh.c
	src/GameSrc/olhscan.c
	src/GameSrc/palfx.c
	src/GameSrc/pathfind.c
	src/GameSrc/physics.c
	src/GameSrc/player.c
	src/GameSrc/plotware.c
	src/GameSrc/popups.c
	src/GameSrc/render.c
	src/GameSrc/rendtool.c
	src/GameSrc/saveload.c
	src/GameSrc/schedule.c
	src/GameSrc/screen.c
	src/GameSrc/setup.c
	src/GameSrc/shodan.c
	src/GameSrc/sideicon.c
	src/GameSrc/sndcall.c
	src/GameSrc/star.c
	src/GameSrc/statics.c
	src/GameSrc/status.c
	src/GameSrc/target.c
	src/GameSrc/textmaps.c
	src/GameSrc/tickcount.c
	src/GameSrc/tfdirect.c
	src/GameSrc/tfutil.c
	src/GameSrc/tools.c
	src/GameSrc/trigger.c
	src/GameSrc/view360.c
	src/GameSrc/viewhelp.c
	src/GameSrc/vitals.c
	src/GameSrc/vmail.c
	src/GameSrc/wares.c
	src/GameSrc/weapons.c
	src/GameSrc/wrapper.c
	src/GameSrc/gamerend.c
	src/GameSrc/mouselook.c
	src/MacSrc/ShockBitmap.c
	src/MacSrc/InitMac.c
	src/MacSrc/Shock.c
	src/MacSrc/Prefs.c
	src/MacSrc/MacTune.c
	src/MacSrc/SDLSound.c
	src/MacSrc/Modding.c
	src/MacSrc/OpenGL.cc
	src/MacSrc/Xmi.c
	src/MusicSrc/MusicDevice.c
)
target_include_directories(systemshock PUBLIC
    src/GameSrc/Headers
    src/MacSrc
    src/MusicSrc
    ${CMAKE_BINARY_DIR}/generated)
target_embedded_shaders_c(systemshock generated/embedded-shaders.c.inc
    shaders/main.vert
    shaders/color.frag
    shaders/star.frag
    shaders/texture.frag
)
target_precompile_headers(systemshock PUBLIC src/GameSrc/Headers/precompiled.h)
if(APPLE)
    target_compile_definitions(systemshock PRIVATE GL_SILENCE_DEPRECATION)
endif()
target_link_libraries(systemshock PUBLIC
    shockLG
    shockRES
    shockDSTRUCT
    shock2D
    shockUI
    shockSND
    shockEDMS
    shockRND
    shockPALETTE
    shockVOX
    shockAFILE
    shockADLMIDI
    # UH-OH breaking a bunch of static linking stuff from upstream! I just want
    # it to build for me for now. And static vs shared deps should be
    # configurable anyway.
    SDL2::SDL2main SDL2::SDL2
    SDL2_mixer::SDL2_mixer
    PkgConfig::fluidsynth
	${OPENGL_LIBRARIES}
	${ALSA_LIBRARIES}
)

# Turn on address sanitizing if wanted
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/externals/sanitizers/cmake" ${CMAKE_MODULE_PATH})
find_package(Sanitizers)
add_sanitizers(systemshock)

# --- Installation Rules ---

install(TARGETS systemshock
	RUNTIME DESTINATION .
)
if(WIN32)
	if(ENABLE_FLUIDSYNTH MATCHES "BUNDLED")
		install(FILES $<TARGET_FILE:libfluidsynth> DESTINATION .)
	endif()
endif()

# Toolchains may define extra libraries that should be included in the distribution.
# This is primarily for MinGW, which needs some runtime libraries along with it.
if(DEFINED TOOLCHAIN_DLL_DEPENDENCIES)
	foreach(DLL ${TOOLCHAIN_DLL_DEPENDENCIES})
		execute_process(
			COMMAND ${CMAKE_C_COMPILER} -print-file-name=${DLL}
			OUTPUT_VARIABLE DLL_PATH
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		if(EXISTS "${DLL_PATH}" AND NOT "${DLL_PATH}" STREQUAL "${DLL}")
			install(FILES "${DLL_PATH}" DESTINATION .)
		else()
			message(WARNING "DLL '${DLL}' not resolved - it will be missing in the package")
		endif()
	endforeach()
endif()

install(FILES
	${CMAKE_SOURCE_DIR}/README.md
	DESTINATION .
)
