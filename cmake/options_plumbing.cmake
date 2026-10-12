# `choice` creates a cache var with attached "allowed" set of string values
# (note CMake doesn't validate these).
# Usage:
#   choice(<var name> "description"
#       CHOICES <choice strings>...
#       VALUE <default value>
#   )
function(choice name desc)
    cmake_parse_arguments(PARSE_ARGV 2 arg
        # flags
        ""
        # single-value
        "VALUE"
        # multi-value
        "CHOICES"
    )
    # auto-append choices to description:
    list(JOIN arg_CHOICES "/" desc_choices)
    set("${name}" "${arg_VALUE}" CACHE STRING "${desc} (${desc_choices})")
    set_property(CACHE "${name}" PROPERTY STRINGS ${arg_CHOICES})
endfunction()

# Plumbing for FeatureSummary. Note this is not enough for showing some
# important non-bool info; may or may not be replaced.
include(FeatureSummary)
# `foption` is `option` but also duplicates it into FeatureSummary.
function(foption name desc)
    option("${name}" "${desc}" ${ARGN})
    add_feature_info("${name}" "${name}" "${desc}")
endfunction()
# `fset` is `set(... CACHE ...)` but also shows it as an enabled feature in
# FeatureSummary if the var is truthy. You *must* pass CACHE in the args or
# else the feature part will get confused trying to extract the description
# text. (The new freeform `set(CACHE{var})` syntax isn't available until 4.2,
# but that would help.)
# macro(fset name)
#     set("${name}" ${ARGN})
#     # set(<variable> <value>... CACHE <type> <docstring> [FORCE])
#     list(GET ARGN -1 desc)
#     if(desc STREQUAL FORCE)
#         list(GET ARGN -2 desc)
#     endif()
#     add_feature_info("${name}" "${name}" "${desc}")
# endmacro()
