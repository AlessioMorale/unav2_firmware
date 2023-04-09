set(additional_compiler_flags "-DLOGGER_RTT -DSEGGER_RTT_MODE_DEFAULT=SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL")
FILE(GLOB rtt_sources "tinyusb/lib/SEGGER_RTT/RTT/*.c")

add_library(rtt STATIC 
    ${rtt_sources}
)
target_compile_options(rtt PUBLIC ${additional_compiler_flags})
target_include_directories(rtt PUBLIC "tinyusb/lib/SEGGER_RTT/RTT")
