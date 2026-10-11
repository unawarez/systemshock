# Generates a .c file containing GLSL files as static constant char arrays.
# The arrays are null-terminated, which happens to be a cute workaround for
# this simple script emitting a trailing comma in the array of byte literals.
#
# This is a detail of shaders.cmake that it uses as a build step via `cmake
# -P`.
#
# CLI:
# * -DSOURCES=<CMake semicolon-list of GLSL file paths>
# * -DOUTPUT=<path to generated C file>

if(NOT SOURCES OR NOT OUTPUT)
    message(FATAL_ERROR "shaders.cmake emitted a bad `cmake -P` call")
endif()
set(content "extern \"C\" {\n\n")
foreach(path IN LISTS SOURCES)
    cmake_path(GET path FILENAME name)
    string(MAKE_C_IDENTIFIER "shader_${name}" c_name)

    file(READ "${path}" hex HEX)
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," hex "${hex}")

    string(APPEND content "static const char ${c_name}[] = {${hex}0x00};\n")
endforeach()
string(APPEND content "}\n") # extern C {...

file(WRITE "${OUTPUT}" "${content}")
