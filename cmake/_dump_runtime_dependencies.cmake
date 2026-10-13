# This is a cmake -P script, an implementation detail of
# dump_runtime_dependencies().
#
# WARNING: this unilaterally forces CMP0207 OLD which means all Windows path regexes need to use [\\/] !
if(NOT BINARY OR NOT OUTPUT)
    message(FATAL_ERROR "dump_runtime_dependencies made a bad `cmake -P` call")
endif()

cmake_policy(SET CMP0207 OLD)

file(GET_RUNTIME_DEPENDENCIES
    RESOLVED_DEPENDENCIES_VAR resolved
    UNRESOLVED_DEPENDENCIES_VAR unresolved
    EXECUTABLES "${BINARY}"
    PRE_INCLUDE_REGEXES ${PRE_INCLUDE_REGEXES}
    PRE_EXCLUDE_REGEXES ${PRE_EXCLUDE_REGEXES}
    POST_INCLUDE_REGEXES ${POST_INCLUDE_REGEXES}
    POST_EXCLUDE_REGEXES ${POST_EXCLUDE_REGEXES}
    POST_INCLUDE_FILES ${POST_INCLUDE_FILES}
    POST_EXCLUDE_FILES ${POST_EXCLUDE_FILES}
)
set(deps ${resolved} ${unresolved})
list(JOIN deps "\n" content)
file(WRITE "${OUTPUT}" "${content}")
