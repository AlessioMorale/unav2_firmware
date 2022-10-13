set(sources
    nanopb/pb_common.c
    nanopb/pb_decode.c
    nanopb/pb_encode.c
)
add_library(nanopb STATIC ${sources})
target_include_directories(nanopb PUBLIC "nanopb")
