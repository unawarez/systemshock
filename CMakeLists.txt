cmake_minimum_required(VERSION 3.29)
project(shockolate VERSION 0.7.8)
# CMake can't handle suffixes in the project version, so this variable is my
# compromise for denoting this is a fork. Should I change the project name
# instead? Maybe.
set(SHOCKOLATE_VERSION_STRING "${PROJECT_VERSION}+unawarez0")
include(cmake/version_append_git_commit.cmake)
version_append_git_commit(SHOCKOLATE_VERSION_STRING)
message(STATUS "Version is ${SHOCKOLATE_VERSION_STRING}")
configure_file("${CMAKE_SOURCE_DIR}/src/GameSrc/Headers/shockolate_version.h.in"
    "${CMAKE_BINARY_DIR}/generated/shockolate_version.h" )

###############################################################################
# External options
###############################################################################
include(cmake/options_plumbing.cmake)
# Main features
foption(SHOCK_ENABLE_OPENGL "Enable OpenGL renderer" ON)
foption(SHOCK_ENABLE_SOUND "Enable sound (requires SDL2_mixer)" ON)
foption(SHOCK_ENABLE_SOUND_FLUIDSYNTH "Enable FluidSynth MIDI playback" ON)
foption(SHOCK_ENABLE_SOUND_ALSA "Enable ALSA MIDI playback on Linux" ON)
# Dev stuff
foption(SHOCK_ENABLE_DEBUGGING_BLIT "Enable debugging blitter" OFF)

# Dependency controls
option(SHOCK_DEPS_DEFAULT_STATIC
    "Static link to dependencies (can be overridden per dependency)"
    ON
)
option(SHOCK_DEPS_DEFAULT_FETCH
    "Download and compile dependency sources (can be overridden per dependency)"
    ON
)

# Declare common 'static vs shared' and 'system vs fetch' vars for normal dependencies.
function(dep_options var_name pretty_name)
    choice(SHOCK_DEPS_${var_name}_STATIC "Static link to ${pretty_name}"
        CHOICES DEFAULT ON OFF
        VALUE DEFAULT
    )
    choice(SHOCK_DEPS_${var_name}_FETCH "Download and compile sources for ${pretty_name}"
        CHOICES DEFAULT ON OFF
        VALUE DEFAULT
    )
endfunction()

dep_options(SDL2 SDL2)
# SDL2_mixer isn't used if SHOCK_ENABLE_SOUND is OFF, but not sure which is
# more troll: the options disappear sometimes, or the options don't disappear
# but sometimes do nothing.
# SDL2_mixer shared/static must match SDL2 shared/static!
# Also, we propagate SDL2_mixer shared/static into SDL2MIXER_DEPS_SHARED to
# control linkage of all its dependencies.
# Double-also, we propagate SDL2_mixer fetch/system into SDL2MIXER_VENDORED to
# control fetch for its dependencies.
dep_options(SDL2_mixer SDL2_mixer)
dep_options(FluidSynth FluidSynth)

###############################################################################
# Dependency setup
###############################################################################
include(FetchContent)

# Get the actual values for a dep's STATIC and FETCH choices: it's kind of
# tricky to collapse `DEFAULT` into the default without accidentally
# overwriting the `DEFAULT` into CMakeCache. This function spoots the vars
# `dep_static`, `dep_shared`, and `dep_fetch` into the parent scope with the
# resolved values. The intended usage is you treat those like scratch vars that
# keep getting overwritten as you set up each dependency.
# TODO could change so that a dep's setup is `function(setup_foo static?
# fetch?)` and meta stuff sets up those args and calls that function. That's
# really not too meta and it's cleaner.
# Also TODO might need to save the resolved values into persistently named vars
# or properties or something, because there are weird conflict cases where we
# could check and give better error messages. For example, if SDL2
# shared/static != SDL2_mixer shared/static, SDL2_mixer configure will always
# die; we could check those vars first and show a messge.
function(resolve_dep_vars name)
    set(dep_static "${SHOCK_DEPS_${name}_STATIC}")
    if(dep_static STREQUAL "DEFAULT")
        set(dep_static "${SHOCK_DEPS_DEFAULT_STATIC}")
    endif()
    # some things want shared instead of static, and inverting a boolean in
    # cmake is dumb, so just do it here.
    set(dep_shared ON)
    if(dep_static)
        set(dep_shared OFF)
    endif()
    set(dep_fetch "${SHOCK_DEPS_${name}_FETCH}")
    if(dep_fetch STREQUAL "DEFAULT")
        set(dep_fetch "${SHOCK_DEPS_DEFAULT_FETCH}")
    endif()
    message(STATUS "${name}: static=${dep_static} fetch=${dep_fetch}")
    return(PROPAGATE dep_static dep_shared dep_fetch)
endfunction()

# SDL2 creates 2 targets: dep_SDL2 and dep_SDL2main. Executables are supposed
# to link SDL2main before SDL2.
resolve_dep_vars(SDL2)
add_library(dep_SDL2 INTERFACE)
add_library(dep_SDL2main INTERFACE)
if(dep_fetch)
    set(SDL_VERSION 2.32.10)
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL
        GIT_TAG "release-${SDL_VERSION}"
        EXCLUDE_FROM_ALL
        # SYSTEM is extra important because on case-insensitive filesystems
        # `#include "sdl_events.h"` will hit a conflict between
        # src/Libraries/INPUT/Source/sdl_events.h and SDL2/SDL_events.h.
        SYSTEM
        OVERRIDE_FIND_PACKAGE
    )
    block()
        set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
        set(SDL_SHARED "${dep_shared}")
        set(SDL_STATIC "${dep_static}")
        set(SDL_TEST OFF)
        set(SDL_INSTALL OFF)
        FetchContent_MakeAvailable(SDL2)
    endblock()

else()
    if(dep_static)
        find_package(SDL2 REQUIRED COMPONENTS SDL2-static SDL2main)
    else()
        find_package(SDL2 REQUIRED COMPONENTS SDL2 SDL2main)
    endif()
endif()
if(dep_static)
    target_link_libraries(dep_SDL2 INTERFACE SDL2::SDL2-static)
else()
    target_link_libraries(dep_SDL2 INTERFACE SDL2::SDL2)
endif()
target_link_libraries(dep_SDL2main INTERFACE SDL2::SDL2main)

# OpenGL
if(SHOCK_ENABLE_OPENGL)
    add_library(dep_opengl INTERFACE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(dep_opengl INTERFACE OpenGL::GL)
    target_compile_definitions(dep_opengl INTERFACE USE_OPENGL=1)
    if(APPLE)
        target_compile_definitions(dep_opengl INTERFACE GL_SILENCE_DEPRECATION)
    endif()
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

        target_include_directories(dep_opengl INTERFACE "${glew_SOURCE_DIR}/include")
        target_link_libraries(dep_opengl glew_s winmm)
	endif(WIN32)
endif()

if(SHOCK_ENABLE_SOUND)
    # meta-target that collects all required + optional sound stuff
    add_library(dep_feature_sound INTERFACE)

    if(SHOCK_ENABLE_SOUND_FLUIDSYNTH)
        resolve_dep_vars(FluidSynth)
        if(dep_fetch)
            set(FLUIDSYNTH_REVISION c539a8d9270ba5a3f7d6e460606483fc2ab1eb61)
            FetchContent_Declare(
                FluidSynth
                GIT_REPOSITORY https://github.com/EtherTyper/fluidsynth-lite.git
                GIT_TAG ${FLUIDSYNTH_REVISION}
                EXCLUDE_FROM_ALL
                SYSTEM
                OVERRIDE_FIND_PACKAGE
            )
            block()
                set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
                set(BUILD_SHARED_LIBS "${dep_shared}")
                # if ( WIN32 )
                #     # Sadly, FluidSynth-Lite has its CMakeLists.txt hardcoded to make its functions as exports.
                #     # Idea: Modify CMakeLists.txt as part of this block here?
                #     set(BUILD_SHARED_LIBS TRUE CACHE BOOL "Force shared libs for FluidSynth on Windows" FORCE)
                # endif ( WIN32 )
                FetchContent_MakeAvailable(FluidSynth)
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
            endblock()
            target_link_libraries(dep_feature_sound INTERFACE libfluidsynth)
        else()
            if(dep_shared)
                find_package(FluidSynth REQUIRED COMPONENTS shared)
            else()
                find_package(FluidSynth REQUIRED COMPONENTS static)
            endif()
            target_link_libraries(dep_feature_sound INTERFACE FluidSynth::libfluidsynth)
        endif()
        target_compile_definitions(dep_feature_sound INTERFACE USE_FLUIDSYNTH=1)
    endif()

    if(LINUX AND SHOCK_ENABLE_SOUND_ALSA)
      # Find ALSA for Linux native MIDI
      # NOTE: this seems to require having 64-bit dev pacakges installed when building
      #  on 64-bit OS, even when building a 32-bit binary
      find_package(ALSA REQUIRED)
      target_link_libraries(dep_feature_sound INTERFACE ALSA::ALSA)
      target_compile_definitions(dep_feature_sound INTERFACE USE_ALSA=1)
    endif()

    # SDL2_mixer CMakeLists contains some fun shit that breaks
    # find_package(FluidSynth), so **FluidSynth setup must come before
    # sdl2_mixer setup!** In fact it might break other packages too. Maybe just
    # try to keep it at the end.
    resolve_dep_vars(SDL2_mixer)
    if(dep_fetch)
        SET(SDL_MIXER_VERSION 2.8.2)
        FetchContent_Declare(
            SDL2_mixer
            GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
            GIT_TAG release-${SDL_MIXER_VERSION}
            EXCLUDE_FROM_ALL
            SYSTEM
            OVERRIDE_FIND_PACKAGE
        )
        block()
            set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
            set(BUILD_SHARED_LIBS "${dep_shared}")
            set(SDL2MIXER_DEPS_SHARED "${dep_shared}")
            set(SDL2MIXER_VENDORED "${dep_fetch}")
            # Manually maintained copypaste of settings declarations from their
            # CMakeLists so that it doesn't download and compile a bunch of stuff.
            set(SDL2MIXER_FLAC OFF)
            set(SDL2MIXER_GME OFF)
            set(SDL2MIXER_MOD OFF)
            set(SDL2MIXER_MP3 OFF)
            # Upstream does not disable main MIDI option, just fluidsynth, so that
            # leaves timidity and sometimes native (on w32/apple/haiku)...there is
            # a strong chance MIDI should be fully disabled.
            set(SDL2MIXER_MIDI_FLUIDSYNTH OFF)
            set(SDL2MIXER_OPUS OFF)
            set(SDL2MIXER_VORBIS "") # not clear if this even works
            set(SDL2MIXER_WAVE OFF)
            set(SDL2MIXER_WAVPACK OFF)
            FetchContent_MakeAvailable(SDL2_mixer)
        endblock()

    else()
        find_package(SDL2_mixer REQUIRED)
    endif()
    if(dep_static)
        target_link_libraries(dep_feature_sound INTERFACE SDL2_mixer::SDL2_mixer-static)
    else()
        target_link_libraries(dep_feature_sound INTERFACE SDL2_mixer::SDL2_mixer)
    endif()
    target_compile_definitions(dep_feature_sound INTERFACE USE_SDL_MIXER=1)

endif()

# Configuration done. Print features
feature_summary(WHAT ENABLED_FEATURES DESCRIPTION "Enabled features:")
feature_summary(WHAT DISABLED_FEATURES DESCRIPTION "Disabled features:")

###############################################################################
# Actual build stuff
###############################################################################

# C/C++ config:
# LANGUAGE_EXTENSIONS OFF is an attempt for non-MSVC devs to avoid writing
# things that will fail on MSVC.
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_LANGUAGE_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_LANGUAGE_EXTENSIONS OFF)

# Codebase was written with these assumptions:
add_compile_options(-fsigned-char -fno-strict-aliasing)

include(cmake/shaders.cmake)

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
target_embedded_shaders_c(systemshock generated/embedded-shaders.c.inc
    shaders/main.vert
    shaders/color.frag
    shaders/star.frag
    shaders/texture.frag
)

target_include_directories(systemshock PUBLIC
    src/GameSrc/Headers
    src/MacSrc
    src/MusicSrc
    ${CMAKE_BINARY_DIR}/generated)
# defined in lg.h, only consumed in GameSrc, but lots of files have a reckless
# assumption that a precompiled header will bring lg.h in scope for them, so:
target_compile_definitions(systemshock PRIVATE SVGA_SUPPORT)
if(SHOCK_ENABLE_DEBUGGING_BLIT)
    target_compile_definitions(systemshock PRIVATE DEBUGGING_BLIT)
endif()
target_precompile_headers(systemshock PUBLIC src/GameSrc/Headers/precompiled.h)

target_link_libraries(systemshock PRIVATE
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
    dep_SDL2main dep_SDL2
)
if(SHOCK_ENABLE_OPENGL)
    target_link_libraries(systemshock PRIVATE dep_opengl)
endif()
if(SHOCK_ENABLE_SOUND)
    target_link_libraries(systemshock PRIVATE dep_feature_sound)
endif()

###############################################################################
# Install rules (HILARIOUSLY BROKEN RN)
###############################################################################

install(TARGETS systemshock
	RUNTIME DESTINATION .
)

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
