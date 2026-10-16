/*

Copyright (C) 2015-2018 Night Dive Studios, LLC.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
//====================================================================================
//
//		System Shock - ©1994-1995 Looking Glass Technologies, Inc.
//
//		Shock.c	-	Mac-specific initialization and main event loop.
//
//====================================================================================

//--------------------
//  Includes
//--------------------
#include <math.h>
#include <SDL.h>
#include <SDL_keyboard.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "InitMac.h"
#include "Modding.h"
#include "OpenGL.h"
#include "Prefs.h"
#include "Shock.h"
#include "ShockBitmap.h"

#include "amaploop.h"
#include "gr2ss.h"
#include "hkeyfunc.h"
#include "log.h"
#include "mainloop.h"
#include "setup.h"
#include "shockolate_version.h"
#include "status.h"
#include "tryhard/files.h"
#include "version.h"

//--------------------
//  Globals
//--------------------
bool gPlayingGame;

grs_screen *cit_screen;
SDL_Window *window;
SDL_Palette *sdlPalette;
SDL_Renderer *renderer;

SDL_AudioDeviceID device;

extern grs_screen *svga_screen;
extern frc *svga_render_context;

//--------------------
//  Prototypes
//--------------------
extern void init_all(void);
extern void inv_change_fullscreen(uchar on);
extern void object_data_flush(void);
extern errtype load_da_palette(void);

// see Prefs.c
extern void CreateDefaultKeybindsFile(void);
extern void LoadHotkeyKeybinds(void);
extern void LoadMoveKeybinds(void);

// CLI arg parsing:
struct ModArg;
struct Args {
    bool no_splash;
    int log_level;
    const char *gamedata_dir;
    const char *savegame_dir;
    const char *soundfonts_dir;
    size_t n_mod_args;
    const char **mod_args;
};
const struct Args DEFAULT_ARGS = {false, LOG_INFO};

static void show_usage(const char *arg0) {
    fprintf(stderr,
            "Usage: %s [options...] [mod file/dir...]\n"
            "Options: (note: long args accept either `-arg` or `--arg`)\n"
            "  -h, -help      Show usage info\n"
            "  -v             Log level DEBUG\n"
            "  -vv            Log level TRACE\n"
            "  -nosplash      Disable splash screen\n"
            "  -gamedir       Path to base game data (res/ folder)\n"
            "  -savedir       Path to save game folder\n"
            "  -soundfontdir  Path to SoundFonts folder for MIDI playback\n",
            arg0);
}

// return 0 => OK, N => show usage and exit with code N-1.
static int parse_args(int argc, char **argv, struct Args *parsed) {
    int result = 0;
    char **args_end = argv + argc;
    for (char **argp = argv + 1; argp < args_end; argp++) {

#define SHIFT_ARG_VALUE(argname)                                              \
    argp++;                                                                   \
    if (argp >= args_end) {                                                   \
        fprintf(stderr, "ERROR: argument `%s` missing a value.\n", #argname); \
        return -2;                                                            \
    } else {                                                                  \
        arg = *argp;                                                          \
    }

        const char *arg = *argp;
        if (arg[0] != '-') {
            // assume it's a mod pathname
            parsed->n_mod_args++;
            parsed->mod_args = realloc(parsed->mod_args, sizeof(char *) * parsed->n_mod_args);
            if (!parsed->mod_args)
                exit(99);
            parsed->mod_args[parsed->n_mod_args - 1] = arg;

        } else if (!strcmp(arg, "-h") || !strcmp(arg, "-help") || !strcmp(arg, "--help")) {
            // weird hack: not returning immediately. parse the rest of the
            // args, and then a) can validate after the -h instead of only
            // before, and b) can show the influence of user selecting stuff
            // (i.e. showing paths, it's maybe(?!) less confusing to show what
            // the user picked instead of the defaults).
            result = 1;
        } else if (!strcmp(arg, "-v")) {
            parsed->log_level = LOG_DEBUG;
        } else if (!strcmp(arg, "-vv")) {
            parsed->log_level = LOG_TRACE;
        } else if (!strcmp(arg, "-nosplash") || !strcmp(arg, "--nosplash")) {
            parsed->no_splash = true;
        } else if (!strcmp(arg, "-gamedir") || !strcmp(arg, "--gamedir")) {
            SHIFT_ARG_VALUE(gamedir);
            parsed->gamedata_dir = arg;
        } else if (!strcmp(arg, "-savedir") || !strcmp(arg, "--savedir")) {
            SHIFT_ARG_VALUE(savedir);
            parsed->savegame_dir = arg;
        } else if (!strcmp(arg, "-soundfontdir") || !strcmp(arg, "--soundfontdir")) {
            SHIFT_ARG_VALUE(soundfontdir);
            parsed->soundfonts_dir = arg;
        } else {
            fprintf(stderr, "ERROR: unknown argument `%s`\n", arg);
            // unlike -h, DO return, because validating the rest may be
            // meaningless.
            return 2;
        }
    }
    return result;
}

//------------------------------------------------------------------------------------
//		Main function.
//------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    struct Args args = DEFAULT_ARGS;

    char *pref_path = SDL_GetPrefPath("Interrupt", "SystemShock");
    if (pref_path) {
        size_t strlen_pref_path = strlen(pref_path);
        // on mac at least, pref path ends in /, but it's not documented and
        // that would be a dumb bug if we appended res without a dir separator.
        if (pref_path[strlen_pref_path - 1] == '/' || pref_path[strlen_pref_path - 1] == '\\') {
            size_t sz = strlen_pref_path + 3 + 1;
            char *res_path = malloc(sz);
            snprintf(res_path, sz, "%sres", pref_path);
            args.gamedata_dir = res_path;
        } else {
            size_t sz = strlen_pref_path + 4 + 1;
            char *res_path = malloc(sz);
            snprintf(res_path, sz, "%s/res", pref_path);
            args.gamedata_dir = res_path;
        }
        args.savegame_dir = pref_path;
        args.soundfonts_dir = pref_path;
        // res_path and pref_path are leaked forever, but, whatever; they live
        // the whole program and can't be changed.
    } else {
        args.gamedata_dir = "./res";
        args.savegame_dir = "./res";
        args.soundfonts_dir = "./res";
    }

    int parse_status = parse_args(argc, argv, &args);
    if (parse_status) {
        show_usage(argv[0]);
        fprintf(stderr,
                "Game paths (according to defaults plus user settings):\n"
                "  Base game data:  %s\n"
                "  Saved games:     %s\n"
                "  SoundFonts:      %s\n",
                args.gamedata_dir, args.savegame_dir, args.soundfonts_dir);
        exit(parse_status - 1);
    }

    shock_files_global_config.gamedata_dir = args.gamedata_dir;
    shock_files_global_config.savegame_dir = args.savegame_dir;
    shock_files_global_config.soundfonts_dir = args.soundfonts_dir;

    // FIXME externalize this
    log_set_quiet(0);
    log_set_level(args.log_level);

    INFO("Logger initialized");

    // init mac managers

    InitMac();

    // Initialize the preferences file.

    SetDefaultPrefs();
    LoadPrefs();

    // see Prefs.c
    CreateDefaultKeybindsFile(); // only if it doesn't already exist
    // even if keybinds file still doesn't exist, defaults will be set here
    LoadHotkeyKeybinds();
    LoadMoveKeybinds();

    // CC: Modding support! This is so exciting.
    InitMods(args.n_mod_args, args.mod_args);

    // Initialize

    init_all();
    setup_init();

    gPlayingGame = true;

    load_da_palette();
    gr_clear(0xFF);

    // Draw the splash screen
    if (!args.no_splash) {
        INFO("Showing splash screen");
        splash_draw();
    }

    // Start in the Main Menu loop

    _new_mode = _current_loop = SETUP_LOOP;
    loopmode_enter(SETUP_LOOP);

    // Start the main loop

    INFO("Showing main menu, starting game loop");
    mainloop(argc, argv);

    status_bio_end();
    stop_music();

    return 0;
}

void InitSDL() {
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0) {
        DEBUG("%s: Init failed", __FUNCTION__);
    }
    // "On desktop platforms, SDL_StartTextInput() is implicitly called on SDL
    // video subsystem initialization"
    SDL_StopTextInput();

    // TODO: figure out some universal set of settings that work...
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);

    gr_init();

    extern short svga_mode_data[];
    gr_set_mode(svga_mode_data[gShockPrefs.doVideoMode], TRUE);

    INFO("Setting up screen and render contexts");

    // Create a canvas to draw to

    SetupOffscreenBitmaps(grd_cap->w, grd_cap->h);

    // Open our window!
    char window_title[128];
    sprintf(window_title, "System Shock - %s", SHOCKOLATE_VERSION);

    window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, grd_cap->w, grd_cap->h,
                              SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);

    // Create the palette

    sdlPalette = SDL_AllocPalette(256);

    // Setup the screen

    svga_screen = cit_screen = gr_alloc_screen(grd_cap->w, grd_cap->h);
    gr_set_screen(svga_screen);

    gr_alloc_ipal();

    SDL_ShowCursor(SDL_DISABLE);

    atexit(SDL_Quit);

    SDL_RaiseWindow(window);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetLogicalSize(renderer, grd_cap->w, grd_cap->h);

    // Startup OpenGL

    init_opengl();

    SDLDraw();

    SDL_ShowWindow(window);
}

SDL_Color gamePalette[256];
bool UseCutscenePalette = FALSE; // see cutsloop.c
void SetSDLPalette(int index, int count, uchar *pal) {
    static bool gammalut_init = 0;
    static uchar gammalut[100 - 10 + 1][256];
    if (!gammalut_init) {
        double factor = (use_opengl() ? 1.0 : 2.2); // OpenGL uses 2.2
        int i, j;
        for (i = 10; i <= 100; i++) {
            double gamma = (double)i * 1.0 / 100;
            gamma = 1 - gamma;
            gamma *= gamma;
            gamma = 1 - gamma;
            gamma = 1 / (gamma * factor);
            for (j = 0; j < 256; j++)
                gammalut[i - 10][j] = (uchar)(pow((double)j / 255, gamma) * 255);
        }
        gammalut_init = 1;
        INFO("Gamma LUT init\'ed");
    }

    int gam = gShockPrefs.doGamma;
    if (gam < 10)
        gam = 10;
    if (gam > 100)
        gam = 100;
    gam -= 10;

    for (int i = index; i < index + count; i++) {
        gamePalette[i].r = gammalut[gam][*pal++];
        gamePalette[i].g = gammalut[gam][*pal++];
        gamePalette[i].b = gammalut[gam][*pal++];
        gamePalette[i].a = 0xff;
    }

    if (!UseCutscenePalette) {
        // Hack black!
        gamePalette[255].r = 0x0;
        gamePalette[255].g = 0x0;
        gamePalette[255].b = 0x0;
        gamePalette[255].a = 0xff;
    }

    SDL_SetPaletteColors(sdlPalette, gamePalette, 0, 256);
    SDL_SetSurfacePalette(drawSurface, sdlPalette);
    SDL_SetSurfacePalette(offscreenDrawSurface, sdlPalette);

    if (should_opengl_swap())
        opengl_change_palette();
}

void SDLDraw() {
    if (should_opengl_swap()) {
        // We want the UI background to be transparent!
        sdlPalette->colors[255].a = 0x00;

        // Draw the OpenGL view
        opengl_swap_and_restore(drawSurface);

        // Set the palette back, and we are done
        sdlPalette->colors[255].a = 0xff;
        return;
    }

    // Clear the screen!
    SDL_RenderClear(renderer);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, drawSurface);

    // Blit to the screen by drawing the surface
    SDL_Rect srcRect = {0, 0, gScreenWide, gScreenHigh};
    SDL_RenderCopy(renderer, texture, &srcRect, NULL);
    SDL_DestroyTexture(texture);

    // Show everything we've drawn
    SDL_RenderPresent(renderer);
}

bool MouseCaptured = FALSE;

extern int mlook_enabled;

void CaptureMouse(bool capture) {
    MouseCaptured = (capture && gShockPrefs.goCaptureMouse);

    if (!MouseCaptured && mlook_enabled && SDL_GetRelativeMouseMode() == SDL_TRUE) {
        SDL_SetRelativeMouseMode(SDL_FALSE);

        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        SDL_WarpMouseInWindow(window, w / 2, h / 2);
    } else
        SDL_SetRelativeMouseMode(MouseCaptured ? SDL_TRUE : SDL_FALSE);
}
