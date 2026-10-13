# This is a cmake -P script, an implementation detail of
# dump_runtime_dependencies().
if(NOT BINARY OR NOT OUTPUT)
    message(FATAL_ERROR "dump_runtime_dependencies made a bad `cmake -P` call")
endif()

file(GET_RUNTIME_DEPENDENCIES
    RESOLVED_DEPENDENCIES_VAR resolved
    UNRESOLVED_DEPENDENCIES_VAR unresolved
    EXECUTABLES "${BINARY}"
)
set(deps ${resolved} ${unresolved})
list(JOIN deps "\n" content)
file(WRITE "${OUTPUT}" "${content}")
