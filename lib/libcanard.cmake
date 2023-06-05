FILE(GLOB sources "libcanard/libcanard/*.c")
add_library(libcanard STATIC ${sources})
target_include_directories(libcanard PUBLIC "libcanard/libcanard")
