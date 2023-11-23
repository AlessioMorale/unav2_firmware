# the name of the target operating system
set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_VERSION   1)
set(CMAKE_SYSTEM_PROCESSOR ARM)


#######################################
set(GCC_PREFIX "arm-none-eabi-")

# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
message("Debug build. $ENV{GCC_PATH}")
set(COMPILER_PATH "")
if(DEFINED ENV{GCC_PATH})
    set(COMPILER_PATH "$ENV{GCC_PATH}/")
endif()

# which compilers to use for C and C++
set(CMAKE_C_COMPILER "${COMPILER_PATH}${GCC_PREFIX}gcc")
set(CMAKE_CXX_COMPILER "${COMPILER_PATH}${GCC_PREFIX}g++")
set(CMAKE_ASM_COMPILER "${COMPILER_PATH}${GCC_PREFIX}gcc")
set(CMAKE_OBJCOPY "${COMPILER_PATH}${GCC_PREFIX}objcopy")
set(CMAKE_OBJDUMP "${COMPILER_PATH}${GCC_PREFIX}objdump")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# cpu
set(CPU "-mcpu=cortex-m4")
# fpu
set(FPU "-mfpu=fpv4-sp-d16")
# float-abi
set(FLOAT-ABI "-mfloat-abi=hard")
# mcu
set(MCU "${CPU} -mthumb ${FPU} ${FLOAT-ABI} -fsingle-precision-constant")
# core flags
set(CORE_FLAGS "${MCU} -mthumb-interwork --specs=nano.specs --specs=nosys.specs ${ADDITIONAL_CORE_FLAGS}")

# compiler: language specific flags
set(CMAKE_C_FLAGS "${CORE_FLAGS} -fno-builtin -Wall -std=c17 -fdata-sections -ffunction-sections  -Wall -fno-common -fstack-usage -g3 -gdwarf-2" CACHE INTERNAL "c compiler flags")
set(CMAKE_C_FLAGS_DEBUG "" CACHE INTERNAL "c compiler flags: Debug")
set(CMAKE_C_FLAGS_RELEASE "" CACHE INTERNAL "c compiler flags: Release")

set(ARM_CFLAGS  "-Wall -Wextra -Wfatal-errors -Wpacked -Winline -Wfloat-equal -Wconversion -Wlogical-op -Wpointer-arith -Wdisabled-optimization -Wno-unused-parameter -Wa,-alh=$(@:.o=.lst) -MMD -MP -MF\"$(@:%.o=%.d)\" -Wall")

set(CMAKE_CXX_FLAGS "${CORE_FLAGS} -fno-rtti -fno-exceptions -fno-builtin -Wall -std=c++17 -fdata-sections -ffunction-sections -g -ggdb3" CACHE INTERNAL "cxx compiler flags")
set(CMAKE_CXX_FLAGS_DEBUG "-g -gdwarf-2" CACHE INTERNAL "cxx compiler flags: Debug")
set(CMAKE_CXX_FLAGS_RELEASE "" CACHE INTERNAL "cxx compiler flags: Release")

set(CMAKE_ASM_FLAGS "${CORE_FLAGS} -x assembler-with-cpp -g -ggdb3 -D__USES_CXX -fdata-sections -ffunction-sections -fno-common" CACHE INTERNAL "asm compiler flags")
set(CMAKE_ASM_FLAGS_DEBUG "" CACHE INTERNAL "asm compiler flags: Debug")
set(CMAKE_ASM_FLAGS_RELEASE "" CACHE INTERNAL "asm compiler flags: Release")

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# find additional toolchain executables
find_program(ARM_SIZE_EXECUTABLE "${COMPILER_PATH}${GCC_PREFIX}size")
find_program(ARM_GDB_EXECUTABLE "${COMPILER_PATH}${GCC_PREFIX}gdb")
find_program(ARM_OBJCOPY_EXECUTABLE "${COMPILER_PATH}${GCC_PREFIX}objcopy")
find_program(ARM_OBJDUMP_EXECUTABLE "${COMPILER_PATH}${GCC_PREFIX}objdump")
