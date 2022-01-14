find_package(Catch2 REQUIRED)

set(SOURCES
    timetest.cpp
    EtherCATTypeStringFormattertest.cpp
    ESIParsertest.cpp
    EtherCATTypeParsertest.cpp
    errorstest.cpp
    IdentityTest.cpp
    DataObjectsTest.cpp
)

add_executable(datatypes_test ${SOURCES})
target_link_libraries(
    datatypes_test
    PRIVATE stdc++fs
            catch2testmain
            datatypes
)
target_include_directories(datatypes_test PUBLIC "${PROJECT_SOURCE_DIR}/src")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/exampleESI.bin
    ${CMAKE_BINARY_DIR}/test_resources/exampleESI.bin
    COPYONLY
)
