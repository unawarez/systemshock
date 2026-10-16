# Build instructions

The project builds a single executable named `systemshock`. It doesn't depend on any files at runtime, other than the game asset files that the user must provide, plus any shared libraries it was compiled against.

Building the project requires CMake version 4.x or late 3.x (see CMakeLists.txt for actual minimum version), plus a C99 compiler and a C++17 compiler. It's a cross-platform codebase, but historically on Windows it required a POSIX compatibility layer (CI builds for this project use MinGW); it may or may not be compatible with MSVC now.

## Quickstart: building for yourself

This assumes familiarity with the command line, but not necessarily with CMake. You'll still need to know how to install CMake and a C/C++ compiler. There are non-command-line options like VS Code that have CMake integration, but require learning how to install and use the tools in their own idiosyncratic way.

The biggest choice is if you want to get dependencies through "fetch mode" or "system mode". Fetch mode downloads and compiles the source code for all dependency libraries (SDL2, SDL2_mixer, etc). System mode searches for the already-compiled dependencies on your system. Fetch mode is less likely to have version conflicts and in many cases will "just work", but it's slower to compile the dependencies, requires an internet connection to do a fresh build from scratch, and you could run into compatibility issues building one of the dependencies on your machine. On the other hand, system mode is faster and leaner if you already have the dependencies installed, but if you have anything missing you will need to know how to diagnose the missing libraries and figure out how to install them on your machine.

For fetch mode:
```
# from the repository root:
cmake --preset fetch
cmake --build build
```

For system mode:
```
# from the repository root:
cmake --preset system
cmake --build build
```

Both the `fetch` and `system` presets use build mode `RelWithDebInfo` (release mode with debug info; for a true debug build, add `-DCMAKE_BUILD_TYPE=Debug` to the command line) and they use `build/` as the default build directory (to override, add `-B mybuilddir` to the command line, and then build with `cmake --build mybuilddir`).

Either way, building will generate an executable at `build/systemshock` (or `build/systemshock.exe` on Windows).

For convenience you can place a `res/` folder with game assets in the repository root, and then running `build/systemshock` from the repo root will let the game find the assets. `/res/` will be ignored by Git, but keep a backup copy elsewhere since `git clean` will delete this one.

> **WARNING:** The CMake install step is old and moderately broken at this point; ignore it for now.

## Advanced CMake configuration

The project declares several settings with names starting with `SHOCK_`. Below is a summary, but the source of truth is the top of CMakeLists.txt (they are all grouped in a section labeled "External options").

### Features

* **SHOCK_ENABLE_OPENGL:** Enable OpenGL renderer. Requires system OpenGL headers/libraries, and GLEW on Windows
* **SHOCK_ENABLE_SOUND:** Enable sound (requires SDL2_mixer)
    * **SHOCK_ENABLE_SOUND_FLUIDSYNTH:** Enable FluidSynth MIDI playback (requires FluidSynth)
    * **SHOCK_ENABLE_SOUND_ALSA:** Enable ALSA MIDI playback on Linux (requires system ALSA headers/libraries)

### Build config

* **SHOCK_BIN_LINK_OPTIONS:** Extra link options for the `systemshock` executable (;-separated list)

### Dependency controls

> **WARNING:** Changing any of these settings after your initial configure is very likely to break your build, so either use `cmake --fresh` or delete your build folder first. On the first configure, most libraries save lots of very config-specific info into the cache, and most of it (especially `find_package` stuff) will not properly recalculate even when you change the "higher-level" settings.

For most (but not all) dependency libraries, you can control if they are downloaded and compiled from source or searched for on the system. You can also control if they are statically or dynamically linked to the game. There are two global settings, SHOCK_DEPS_DEFAULT_FETCH and SHOCK_DEPS_DEFAULT_STATIC, that control this for all libraries, and then you can override these settings per library by setting `SHOCK_DEPS_<name>_FETCH` or `SHOCK_DEPS_<name>_STATIC`. Note these settings are all binding requirements; there is no automatic fallback logic if libraries aren't found or can't be built. The allowed values are:

* **FETCH:** ON means "download source code from a fixed URL and compile it". OFF means "find the library on your system", usually via CMake's `find_package`. You may be able to use standard CMake flags to control the `find_package` search, or else directly override the discovered paths in the CMake cache.
* **STATIC:** ON means "static link to this library". OFF means "dynamic link to this library".

General guidelines for fetch and static settings:

| FETCH | STATIC | Description
|-------|--------|-------------
| ON    | ON     | Self-contained executable. Good for Windows and Mac-app-bundle releases. Difficult for Linux distribution because graphics dependencies are very messy.
| ON    | OFF    | Unlikely to have any advantage over ON/ON.
| OFF   | OFF    | Use system shared libraries. Good for Linux distro-specific or Homebrew releases.
| OFF   | ON     | Unlikely to work because many system/Homebrew packages don't provide static libraries.

Per-library control is intended as an escape hatch for environment-specific problems. Some per-library choices may cause conflicts. For example, it is known that SDL2 and SDL2_mixer must have the same STATIC setting or else they will fail to build. It's assumed if you're at this level of config, you know what you're doing and how to diagnose errors.

Allowed names for the `<name>` part of the per-library overrides (case-sensitive):
* `SDL2`
* `SDL2_mixer`
* `FluidSynth`

Note that list excludes OpenGL and ALSA; they cannot be fetched or statically linked (TODO can you static link ALSA?). (Also GLEW is a weird secret dependency specifically for Windows; it is always fetched and compiled. Mac and Linux currently get away with not having a GL loader.)

Fetch mode operates on fixed URLs that can't be overridden. The fetch code may override some build settings of the libraries it builds. Defining subproject settings yourself on the main CMake project may or may not be passed through correctly (this could probably be improved if needed).
