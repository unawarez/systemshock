# Generates a text file at build time containing a target's runtime
# dependencies, as reported by file(RUNTIME_DEPENDENCIES).
#
# output_filename is relative to target's binary dir.
#
# TODO target must be an executable specifically because
# file(GET_RUNTIME_DEPENDENCIES) needs to be told if it's an exe or a library.
# so that info would need to be passed through to the -P script to support
# libraries.
function(dump_runtime_dependencies target output_filename)
    cmake_parse_arguments(PARSE_ARGV 2 arg
        ""
        "SAVE_FILENAME_AS"
        "PRE_INCLUDE_REGEXES;PRE_EXCLUDE_REGEXES;POST_INCLUDE_REGEXES;POST_EXCLUDE_REGEXES;POST_INCLUDE_FILES;POST_EXCLUDE_FILES"
    )
    get_target_property(assumed_out_dir "${target}" BINARY_DIR)
    cmake_path(ABSOLUTE_PATH output_filename BASE_DIRECTORY "${assumed_out_dir}")

    add_custom_command(TARGET "${target}" POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
            "-DBINARY=$<TARGET_FILE:${target}>"
            "-DOUTPUT=${output_filename}"
            "-DPRE_INCLUDE_REGEXES=${arg_PRE_INCLUDE_REGEXES}"
            "-DPRE_EXCLUDE_REGEXES=${arg_PRE_EXCLUDE_REGEXES}"
            "-DPOST_INCLUDE_REGEXES=${arg_POST_INCLUDE_REGEXES}"
            "-DPOST_EXCLUDE_REGEXES=${arg_POST_EXCLUDE_REGEXES}"
            "-DPOST_INCLUDE_FILES=${arg_POST_INCLUDE_FILES}"
            "-DPOST_EXCLUDE_FILES=${arg_POST_EXCLUDE_FILES}"
            -P "${CMAKE_SOURCE_DIR}/cmake/_dump_runtime_dependencies.cmake"
        BYPRODUCTS "${output_filename}"
        VERBATIM
    )

    if(arg_SAVE_FILENAME_AS)
        set("${arg_SAVE_FILENAME_AS}" "${output_filename}" PARENT_SCOPE)
    endif()
endfunction()
