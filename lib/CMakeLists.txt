include("hal_driver.cmake")
include("cmsis.cmake")

set(GIT_DIR_LOOKUP_POLICY ALLOW_LOOKING_ABOVE_CMAKE_SOURCE_DIR)
add_subdirectory("etl")
include("nanopb.cmake")

add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM
  INTERFACE FREERTOS_CONFIG_INCLUDE) # FREERTOS_CONFIG_INCLUDE The config file directory

# target_compile_definitions(freertos_config
#   PUBLIC
#   projCOVERAGE_TEST=0)

add_subdirectory("FreeRTOS-Kernel")
include("tinyusb.cmake")
include("embedded_cli.cmake")
include("rtt.cmake")
