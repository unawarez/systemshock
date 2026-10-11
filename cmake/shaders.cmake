# Usage:
#   target_embedded_shaders_c(
#       consuming_target
#       generated_file.c.inc
#       file1.glsl
#       somepath/file2.glsl
#       ...
#   )
# That example will generate a C file that defines `static const char
# shader_file1[]` and `static const char shader_file2[]`. The arrays are
# null-terminated.
#
# THE OUTPUT FILE EXTENSION WILL CAUSE BAD MAGIC TO HAPPEN IF IT IS .c! (This
# has to emit into target sources to make the build actually work, but then if
# the path ends in .c, CMake will enter it into compilation and linking.)
#
# Filenames are turned into C var names as `shader_` + CMake's
# MAKE_C_IDENTIFIER of the filename (no path, but yes extension).
#
# Consuming target is needed to make CMake emit a build dependency; otherwise
# this might never run.
#
# Consuming code will have to #include the .c file. It's weird and gross but
# it's a quick way to avoid some linker issues/suboptimality. (The "correct"
# way is to generate a .h next to the .c and add the .c as a source file to the
# consuming target. But then dumb things can happen with name mangling and
# symbol visibility, so would need to deal with all that.)
function(target_embedded_shaders_c consuming_target outfile)
    set(genscript "${CMAKE_SOURCE_DIR}/cmake/_generate_embedded_shaders.cmake")
    cmake_path(ABSOLUTE_PATH outfile BASE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    add_custom_command(
        OUTPUT "${outfile}"
        DEPENDS "${genscript}" ${ARGN}
        COMMAND ${CMAKE_COMMAND}
            "-DSOURCES=${ARGN}"
            "-DOUTPUT=${outfile}"
            -P "${genscript}"
        # force workdir is an easy hack around absolute'izing all the input paths
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Generating embedded shaders file"
        VERBATIM
    )
    target_sources("${consuming_target}" PRIVATE "${outfile}")
endfunction()
