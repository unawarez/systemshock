add_library(tryhard STATIC
    files.cpp
)
target_include_directories(tryhard PUBLIC pub)
target_include_directories(tryhard PRIVATE pub/tryhard)
