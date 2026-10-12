function(version_append_git_commit var)
    # idk what happens if you return(PROPAGATE) something you never set, so:
    set("${var}" "${${var}}")
    find_package(Git)
    if(GIT_FOUND)
        # --exclude=* ignores tags, so the output is always just the commit hash
        execute_process(COMMAND ${GIT_EXECUTABLE} describe --always --dirty --exclude=*
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE git_describe_out
            ERROR_VARIABLE git_describe_error
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE git_describe_result
        )
        if(git_describe_result EQUAL 0)
            string(APPEND "${var}" "-g${git_describe_out}")
        else()
            message(WARNING "git describe failed: ${git_describe_error}")
        endif()
    endif(GIT_FOUND)
    return(PROPAGATE "${var}")
endfunction()
