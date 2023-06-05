file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/transpiled)

execute_process(
        COMMAND git rev-parse --short=16 HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE vcs_revision_id
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "vcs_revision_id: ${vcs_revision_id}")
add_definitions(
        -DVCS_REVISION_ID=0x${vcs_revision_id}ULL
        -DNODE_NAME="org.pizzarobotics.unav2"
)
# building Cyphal data type headers
find_package(nnvg REQUIRED)

create_dsdl_target(             # Generate the support library for generated C headers, which is "nunavut.h".
        "nunavut_support"
        c
        ${CMAKE_BINARY_DIR}/transpiled
        ""
        OFF
        little
        "only"
)

set(dsdl_root_namespace_dirs                # List all DSDL root namespaces to transpile here.
        ${LIBS_FOLDER}/public_regulated_data_types/uavcan
        ${LIBS_FOLDER}/public_regulated_data_types/reg
)

foreach (ns_dir ${dsdl_root_namespace_dirs})
    get_filename_component(ns ${ns_dir} NAME)
    message(STATUS "DSDL namespace ${ns} at ${ns_dir}")
    create_dsdl_target(
            "dsdl_${ns}"                    # CMake target name
            c                               # Target language to transpile into
            ${CMAKE_BINARY_DIR}/transpiled  # Destination directory (add it to the includes)
            ${ns_dir}                       # Source directory
            OFF                             # Disable variable array capacity override
            little                          # Endianness of the target platform (alternatives: "big", "any")
            "never"                         # Support files are generated once in the nunavut_support target (above)
            ${dsdl_root_namespace_dirs}     # Look-up DSDL namespaces
    )
    message(STATUS "${CMAKE_BINARY_DIR}/transpiled")

    add_dependencies("dsdl_${ns}" nunavut_support)
endforeach ()

include_directories(SYSTEM ${CMAKE_BINARY_DIR}/transpiled)  # Make the transpiled headers available for inclusion.
add_definitions(-DNUNAVUT_ASSERT=assert)
add_dependencies(${elf_file} dsdl_uavcan dsdl_reg)
