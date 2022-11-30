FILE(GLOB sources "tinyusb/src/*.c")
FILE(GLOB sources_class "tinyusb/src/class/**/*.c")
FILE(GLOB sources_device "tinyusb/src/device/*.c")
FILE(GLOB sources_common "tinyusb/src/common/*.c")
#FILE(GLOB sources_portable "tinyusb/src/portable/st/synopsys/*.c")
FILE(GLOB sources_portable "tinyusb/src/portable/synopsys/dwc2/*.c")

list(APPEND sources
    ${sources_class}
    ${sources_device}
    ${sources_common}
    ${sources_portable}
)
add_library(tinyusb STATIC ${sources})
target_link_libraries(tinyusb PUBLIC freertos_kernel cmsis)
target_include_directories(tinyusb PUBLIC "tinyusb/src")
