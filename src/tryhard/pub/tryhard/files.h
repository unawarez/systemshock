#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

typedef struct ShockFilesConfig {
    // The 'res/' path, but not named 'res' here because that name is very
    // overloaded by the actual LG Res file format and assoc'd subsystem.
    const char *gamedata_dir;
    // Save game dir. CurrentGame.dat is also placed here (it's a runtime thing
    // critical for the game).
    const char *savegame_dir;
} ShockFilesConfig;

// cute lil unsafe global so existing code can be moved over without changing
// everyone's API to pipe through these cfg structs:
extern ShockFilesConfig shock_files_global_config;

// Open a path relative to the game data dir. Sort of. Below behavior was
// created exactly so that ResOpenFile would work in all the ways it is called
// in the codebase today.
//
// If `path` is relative: open the file relative to `cfg->gamedata_dir`, *and*
// do an ASCII-case-insensitive fallback search within `gamedata_dir`
// (including subdir names as well as the filename). This is the normal
// compatibility path for ResOpenFile and other static game asset loading,
// **but** callers should *not* put `res/` at the front of their paths.
//
// If `path` is absolute: just open `path` directly; ignore
// `cfg->gamedata_dir`, and don't do case-insensitive fallback.
//
// Again re: being designed by ResOpenFile usage, almost all ResOpenFile calls
// are to load game assets from the game assets directories, but savegames are
// also LG Res files, and they get opened through ResOpenFile, so making the
// callers pass absolute vs relative paths is the only way to make the
// shock_files_open_gamedata call buried inside ResOpenFile to behave in 2
// different ways without modifying ResOpenFile's API (or moving all the
// callers away from it).
//
// `path` must be UTF-8.
//
// WARNING: if you are using write/create modes, this will just write the file
// without checking for case-insensitive matches already existing.
//
// fun new API pattern: null cfg means use the global pointer.
FILE *shock_files_open_gamedata(const ShockFilesConfig *cfg, const char *path_utf8, const char *mode);

// Path logic of the prev function but returns an absolute path instead of the
// fopen. Free with free(). NOTE non-null does not guarantee the file exists!
// This doesn't check absolute paths. I guess it could, but you should be
// checking your file op error codes anyway. Also TOCTOU.
char *shock_files_resolve_gamedata(const ShockFilesConfig *cfg, const char *path);

// Turn a savegame name like `savgam00.dat` into an absolute path. Optionally
// check if the path exists.
//
// Save games are ultimately read/written via ResOpenResFile, so, given how
// that currently works, save paths must be absolute or they'll get resolved as
// gamedata paths. But there's an outer layer where gamewrap.c uses `copy_file`
// as the first step of loading and the last step of saving; `copy_file` needs
// to see the actual path as well (the code before this relied on
// `fopen_caseless` to resolve it into something).
//
// If `check_stat` is false, then this function just computes the path and
// returns it; it can't fail unless there is some catastrophic internal
// failure inside C++ std::filesystem doing path manipulation. But if
// `check_stat` is true, this function returns null if the path does not point
// to a regular file.
//
// A non-null return value is a malloc'd UTF-8-encoded absolute path to the
// file, which can then be passed to ResOpenResFile and it will be found
// correctly.
char *shock_files_resolve_savegame(const ShockFilesConfig *cfg, const char *save_filename, bool check_stat);

// Calculate the path of CurrentGame.dat. For now it is placed in the savegame
// directory. Possibly it should live in a temp folder instead (and
// double-possibly the game should be cleaning it up, or using move instead of
// copy...)
//
// Returns a malloc'd UTF-8-encoded absolute path to the file.
char *shock_files_resolve_currentgame(const ShockFilesConfig *cfg);

#ifdef __cplusplus
} // extern C
#endif
