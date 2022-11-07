cmake_minimum_required(VERSION 3.10)

project(CS537-Project3A)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 99)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_library(
    ppsort STATIC
    ${PROJECT_SOURCE_DIR}/libpsort.c
)

include_directories(${PROJECT_SOURCE_DIR})

add_executable(
    psort
    main.c
)

target_link_libraries(
    psort
    ppsort
    pthread
)

add_executable(
    printkey
    printkey.c
)

target_link_libraries(
    printkey
    ppsort
)

add_executable(
    gentest
    ${PROJECT_SOURCE_DIR}/gentest.c
)
