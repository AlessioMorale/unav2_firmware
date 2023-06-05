include("hal_driver.cmake")
include("cmsis.cmake")

set(GIT_DIR_LOOKUP_POLICY ALLOW_LOOKING_ABOVE_CMAKE_SOURCE_DIR)
add_subdirectory("etl")

add_subdirectory("FreeRTOS-Kernel") 
include("tinyusb.cmake")
include("libcanard.cmake")
include("o1heap.cmake")
include("rtt.cmake")
