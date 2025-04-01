cmake_minimum_required(VERSION 3.31.6)
project(pump-server VERSION 0.1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(open62541 REQUIRED)

# Generate source from nodesets

set(SRC_OUTPUT_DIR "${PROJECT_SOURCE_DIR}/gen")
set(UA_NODESET_DIR "${PROJECT_SOURCE_DIR}/UA-Nodeset")

# Generate the types and namespace for DI -------------------------------------------------------------------------------

ua_generate_nodeset_and_datatypes(
    NAME "di"
    FILE_CSV "${UA_NODESET_DIR}/DI/Opc.Ua.Di.NodeIds.csv"
    FILE_BSD "${UA_NODESET_DIR}/DI/Opc.Ua.Di.Types.bsd"
    FILE_NS "${UA_NODESET_DIR}/DI/Opc.Ua.Di.NodeSet2.xml"
    OUTPUT_DIR "${SRC_OUTPUT_DIR}"
)

add_library(ua_gen_di
    ${UA_NODESET_DI_SOURCES}
    ${UA_NODESET_DI_HEADERS}
    ${UA_TYPES_DI_SOURCES}
    ${UA_TYPES_DI_HEADERS}
)
target_link_libraries(ua_gen_di PUBLIC open62541::open62541)

# Generate the namespace for Machinery ----------------------------------

ua_generate_nodeset(
    NAME "machinery"
    FILE "${UA_NODESET_DIR}/Machinery/Opc.Ua.Machinery.NodeSet2.xml"
    DEPENDS_TYPES
        "UA_TYPES" "UA_TYPES_DI"
    DEPENDS_NS
        "${UA_NODESET_DIR}/Schema/Opc.Ua.NodeSet2.xml"
        "${UA_NODESET_DIR}/DI/Opc.Ua.Di.NodeSet2.xml"
    DEPENDS_TARGET "open62541-generator-ns-di"
    OUTPUT_DIR "${SRC_OUTPUT_DIR}"
)

add_library(ua_gen_machinery
    ${UA_NODESET_MACHINERY_SOURCES}
    ${UA_NODESET_MACHINERY_HEADERS}
)
target_link_libraries(ua_gen_machinery PUBLIC ua_gen_di)

# Generate the types and namespace for Pumps ------------------------------------------------------

ua_generate_nodeset_and_datatypes(
    NAME "pumps"
    FILE_NS "${UA_NODESET_DIR}/Pumps/Opc.Ua.Pumps.NodeSet2.xml"
    FILE_CSV "${UA_NODESET_DIR}/Pumps/Opc.Ua.Pumps.NodeSet2.csv" 
    FILE_BSD "${UA_NODESET_DIR}/Pumps/Opc.Ua.Pumps.NodeSet2.bsd" 
    OUTPUT_DIR "${SRC_OUTPUT_DIR}"
    DEPENDS machinery
)

add_library(ua_gen_pumps
    ${UA_NODESET_PUMPS_SOURCES}
    ${UA_NODESET_PUMPS_HEADERS}
    ${UA_TYPES_PUMPS_SOURCES}
    ${UA_TYPES_PUMPS_HEADERS}
    "${SRC_OUTPUT_DIR}/pumps_nodeids.h"
)
target_link_libraries(ua_gen_pumps PUBLIC ua_gen_di)
target_link_libraries(ua_gen_pumps PUBLIC ua_gen_machinery)

# -------------------------------------------------------------------

set(SOURCE_FILES "main.c" "${PROJECT_SOURCE_DIR}/src/example.c")

add_executable("${PROJECT_NAME}" ${SOURCE_FILES})

target_include_directories("${PROJECT_NAME}" 
    PRIVATE 
        "${PROJECT_BINARY_DIR}" 
        "${PROJECT_SOURCE_DIR}/src"
        "${SRC_OUTPUT_DIR}"
)

target_link_libraries(
    "${PROJECT_NAME}" 
    open62541::open62541
    ua_gen_di
    ua_gen_machinery
    ua_gen_pumps
)

install(TARGETS "${PROJECT_NAME}" DESTINATION bin)
