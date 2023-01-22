set(sources
    embedded-cli/lib/src/embedded_cli.c
)
add_library(embedded_cli STATIC ${sources})
target_include_directories(embedded_cli PUBLIC "embedded-cli/lib/include")
