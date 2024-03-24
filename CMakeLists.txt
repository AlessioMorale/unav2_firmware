cmake_minimum_required(VERSION 3.0)
include("./CMake/GNU-ARM-Toolchain.cmake")
include_directories("app/include")
add_compile_definitions(LOGGER "rtt")
add_compile_definitions(ENABLE_LOGGER)
# set the build type
if(NOT CMAKE_BUILD_TYPE) 
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Debug build.")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
  message("Release build.")
endif()

set(LIBS_FOLDER ${CMAKE_SOURCE_DIR}/lib)

# set search path for CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)

#generic settings
# Speed of external resonator
set(HSE_VALUE 8000000 CACHE STRING "")

# include OpenOCD module, but make it not required
find_package(Openocd)

# Project specific settings
set(application_name "uNav2")

set(PROJECT_VERSION "0.0.99")
if(DEFINED ENV{UNAV_PROJECT_VERSION})
    set(PROJECT_VERSION "$ENV{UNAV_PROJECT_VERSION}/")
endif()
project (${application_name} VERSION ${PROJECT_VERSION} LANGUAGES C CXX ASM)

# FreeRTOS settings
# get_filename_component(ABSOLUTE_PATH ${RELATIVE_PATH} ABSOLUTE)
get_filename_component(FREERTOS_CONFIG_FILE_DIRECTORY "./app/include/" ABSOLUTE)
set(FREERTOS_PORT "GCC_ARM_CM4F" CACHE STRING "")

# TinyUSB settings

# STM32-specific settings
set(DEVICE_FAMILY STM32F40_41xxx CACHE STRING "")
add_compile_definitions(STM32F405xx)

# Used linker file (just for testing)
get_filename_component(LINKER_SCRIPT linker/STM32F405RGTx_FLASH.ld ABSOLUTE)

# Use LTO? WARNING: This might break the code
#set(ADDITIONAL_CORE_FLAGS -flto)

# OpenOCD config file
get_filename_component(OPENOCD_CONFIG "tools/openocd.cfg" ABSOLUTE)

# add libraries
add_subdirectory(lib)

# add main project
add_subdirectory("app")
