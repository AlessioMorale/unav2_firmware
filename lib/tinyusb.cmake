FILE(GLOB sources "tinyusb/src/*.c")
add_library(tinyusb STATIC ${sources})
target_link_libraries(tinyusb PUBLIC freertos_kernel)
target_include_directories(tinyusb PUBLIC "tinyusb/src")
