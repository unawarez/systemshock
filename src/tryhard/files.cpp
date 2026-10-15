#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "files.h"

// Everything in this file does tons of stupid allocations and string copies
// thanks to c++17's std::filesystem API making every single little thing a
// full string copy, but the alternative is spend a month writing and debugging
// pure-c/c++ cross-platform path logic, or mandate POSIX/MinGW forever and
// spend a week writing it in that, or add another third-party lib, or add
// a less-hell language like rust/zig/etc to the project's build dependencies.
// File path stuff is not a hot path in a game, so for now I don't care.

namespace stdfs = std::filesystem;
using stdpath = std::filesystem::path;
using pathstring = std::filesystem::path::string_type;

// acceptable defaults instead of segfaults:
ShockFilesConfig shock_files_global_config{"res", "savegame"};

// Replacement for std::tolower used in the case-insensitive searching.
// Everything online says std::tolower is full of undefined behavior. The case
// insensitivity in these functions only cares about ASCII because it's really
// for DOS-era compatibility with the asset files (and may not even be relevant
// anymore). It's written as a template to hopefully be tolerant of actual
// windows paths if this was ever compiled with msvc instead of mingw; windows
// paths probably still maybe sometimes possibly use UCS2 encoding.
template <class Char> static Char ascii_tolower(Char ch) {
    return (ch >= 'A' && ch <= 'Z') ? Char(ch - 'A' + 'a') : ch;
}

// so...looking through case-insensitive dir matches is technically an
// iterator-shaped problem, because there could be foo, Foo, FoO, fOO...
// so this thing is a class-that-looks-like-a-function-via-its-constructor,
// whose instances support ranged-for (just needs to provide begin() and
// end()).
class case_insensitive_file_matches {
    stdfs::directory_iterator di;
    pathstring filename_lowercase{};

  public:
    class iterator {
        stdfs::directory_iterator di;
        pathstring filename_lowercase;

        // always performs a test, but might not move forward. that makes it
        // usable as step 1 of a freshly constructed instance. but means it's
        // someone else's responsibility to move forward when the user wants to
        // move forward, else it just keeps testing the same match over and
        // over.
        void settle() {
            const stdfs::directory_iterator end{};
            for (; di != end; di++) {
                const auto entry{*di};
                // entry.path() is absolute(ish?) so name portion must be stripped out:
                const stdpath test_name{entry.path().filename()};
                const auto test_name_2_faster_furiouser{test_name.native()}; // test_name memory must be kept alive

                if (test_name_2_faster_furiouser.size() == filename_lowercase.size()) {
                    bool ok_so_far{true};
                    auto lhs = test_name_2_faster_furiouser.begin();
                    auto lhs_end = test_name_2_faster_furiouser.end();
                    auto rhs = filename_lowercase.begin();
                    auto rhs_end = filename_lowercase.end();
                    for (; ok_so_far && lhs != lhs_end && rhs != rhs_end; lhs++, rhs++) {
                        if (ascii_tolower(*lhs) != *rhs) {
                            ok_so_far = false;
                            break;
                        }
                    }
                    if (ok_so_far) {
                        return;
                    }
                }
            }
        }

        // always moves forward (unless at end) without looking at current
        // state at all.
        void advance() {
            if (di != stdfs::directory_iterator{}) {
                ++di;
            }
        }

      public:
        iterator(stdfs::directory_iterator di, pathstring filename_lowercase)
            : di{di}, filename_lowercase{filename_lowercase} {
            // the result of begin() must be "falsy/end-y" IMMEDIATELY if there
            // are no matches; can't wait until operator++ to do logic.
            settle();
        }

        operator bool() const { return di != stdfs::directory_iterator{}; }

        bool operator==(const iterator &rhs) { return di == rhs.di; }

        iterator &operator++() {
            advance();
            settle();
            return *this;
        }

        const stdfs::directory_entry &operator*() const { return *di; }
    };

  public:
    case_insensitive_file_matches(const stdpath &dir, pathstring filename) : filename_lowercase{std::move(filename)} {
        di = stdfs::directory_iterator{dir};
        std::transform(filename_lowercase.begin(), filename_lowercase.end(), filename_lowercase.begin(),
                       [](auto ch) { return ascii_tolower(ch); });
    }

    iterator begin() { return iterator{di, filename_lowercase}; }
    iterator end() { return iterator{{}, filename_lowercase}; }
};

// recursive version of the above that case-insensitively matches each path
// component in the path being searched for.
class case_insensitive_tree_matches {
    class iterator {
        std::vector<case_insensitive_file_matches::iterator> walk_stack;
        stdpath::iterator search_path_next;
        stdpath::iterator search_path_end;

        void settle() {
            while (!walk_stack.empty()) {
                auto &current_walk(walk_stack.back());
                if (current_walk) {
                    if (search_path_next == search_path_end) {
                        // we are sitting on a full end-to-end match right now.
                        return;
                    } else {
                        auto new_walk_dir{(*current_walk).path()};
                        auto new_walk_filename{*search_path_next};
                        ++current_walk;
                        ++search_path_next;
                        // put this at the very end becuase it invalidates the walk object we're looking at!
                        walk_stack.push_back(case_insensitive_file_matches(new_walk_dir, new_walk_filename).begin());
                    }
                } else {
                    // dir iter exhausted
                    --search_path_next;
                    walk_stack.pop_back();
                }
            }
        }

        void advance() {
            if (!walk_stack.empty()) {
                ++walk_stack.back();
            }
        }

      public:
        iterator() {}
        iterator(stdpath base, const stdpath &search_path) {
            auto sp_begin{search_path.begin()}, sp_end{search_path.end()};
            if (sp_begin != sp_end) {
                walk_stack = {{case_insensitive_file_matches(base, *sp_begin).begin()}};
                auto sp_it{sp_begin};
                search_path_next = ++sp_it;
                search_path_end = sp_end;
                settle();
            }
        }

        operator bool() const { return !walk_stack.empty(); }

        bool operator==(const iterator &rhs) { return walk_stack == rhs.walk_stack; }

        iterator &operator++() {
            advance();
            settle();
            return *this;
        }

        const stdfs::directory_entry &operator*() const { return *walk_stack.back(); }
    };

    // all that lives in the range class itself is holding onto these so we can
    // construct an `iterator` inside begin().
    stdpath base;
    stdpath search_path;

  public:
    case_insensitive_tree_matches(stdpath base, stdpath search_path) : base{base}, search_path{search_path} {}

    iterator begin() { return iterator{base, search_path}; }
    iterator end() { return iterator{}; }
};

#define DONT_UNWIND(failvalue, ...)                 \
    try {                                           \
        __VA_ARGS__                                 \
    } catch (const std::exception &e) {             \
        /* TODO log but headers are being stupid */ \
        return failvalue;                           \
    } catch (...) { /* TODO more dire message */    \
        return failvalue;                           \
    }

char *mallocize_byte_string(const std::string &str) {
    char *out = (char *)malloc(str.size() + 1);
    snprintf(out, str.size() + 1, "%s", str.c_str());
    return out;
}

FILE *shock_files_open_gamedata(const ShockFilesConfig *cfg, const char *path, const char *mode) {
    DONT_UNWIND(nullptr, {
        if (!cfg) {
            cfg = &shock_files_global_config;
        }

        const stdpath input{stdfs::u8path(path)};
        if (input.empty()) {
            throw std::runtime_error{"path is empty"};
        } else if (input.is_absolute()) {
            return fopen(input.c_str(), mode);
        } else {
            auto pathfinder{case_insensitive_tree_matches{stdfs::u8path(cfg->gamedata_dir), input}.begin()};
            if (pathfinder) {
                return fopen((*pathfinder).path().c_str(), mode);
            } else {
                return nullptr;
            }
        }
    })
}

char *shock_files_resolve_gamedata(const ShockFilesConfig *cfg, const char *path) {
    DONT_UNWIND(nullptr, {
        if (!cfg) {
            cfg = &shock_files_global_config;
        }

        // TODO uncopypaste with above
        const stdpath input{stdfs::u8path(path)};
        if (input.empty()) {
            throw std::runtime_error{"path is empty"};
        } else if (input.is_absolute()) {
            return mallocize_byte_string(input.u8string());
        } else {
            auto pathfinder{case_insensitive_tree_matches{stdfs::u8path(cfg->gamedata_dir), input}.begin()};
            if (pathfinder) {
                return mallocize_byte_string((*pathfinder).path().u8string());
            } else {
                return nullptr;
            }
        }
    })
}

char *shock_files_resolve_savegame(const ShockFilesConfig *cfg, const char *save_filename, bool check_stat) {
    DONT_UNWIND(nullptr, {
        if (!cfg) {
            cfg = &shock_files_global_config;
        }

        stdpath test_path{stdfs::absolute(stdfs::u8path(cfg->savegame_dir))};
        test_path /= stdfs::u8path(save_filename);

        if (!check_stat) {
            return mallocize_byte_string(test_path.u8string());
        } else {
            auto status{stdfs::status(test_path)};
            if (!stdfs::exists(status)) {
                return nullptr;
            } else if (!stdfs::is_regular_file(status)) {
                // TODO log that it's weird
                return nullptr;
            } else {
                return mallocize_byte_string(test_path.u8string());
            }
        }
    })
}

char *shock_files_resolve_currentgame(const ShockFilesConfig *cfg) {
    DONT_UNWIND(nullptr, {
        if (!cfg) {
            cfg = &shock_files_global_config;
        }

        stdpath path{stdfs::absolute(stdfs::u8path(cfg->savegame_dir))};
        // lol dumb hack that needs to happen somewhere else
        stdfs::create_directories(path);
        path /= "CurrentGame.dat";
        return mallocize_byte_string(path.u8string());
    })
}
