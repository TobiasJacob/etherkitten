cmake_minimum_required(VERSION 3.10)

project(EtherKITten)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Whether to build the Catch2 unit tests" ON)

option(ENABLE_RT "Whether to enable thread pinning and priority adjustment when reading data" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# When compiling with clang, these flags seem to be necessary for unknown reasons.
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++ -lm")

if(BUILD_TESTING AND (NOT CMAKE_BUILD_TYPE OR (CMAKE_BUILD_TYPE EQUAL "DEBUG")))
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fprofile-arcs -ftest-coverage") # enable Gcov
endif()

add_subdirectory(datatypes)
add_subdirectory(reader)
add_subdirectory(gui)
add_subdirectory(config)
add_subdirectory(controller)
if(BUILD_TESTING)
    add_subdirectory(mocks)
    add_subdirectory(test)
endif()
