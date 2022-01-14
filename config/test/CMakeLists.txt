find_package(Catch2 REQUIRED)

set(SOURCES
    iotest.cpp
    BusConfigtest.cpp
    BusLayouttest.cpp
)

set(HEADERS ConfigObserverDummy.hpp)

add_executable(config_test ${SOURCES} ${HEADERS})
target_link_libraries(
    config_test
    PUBLIC stdc++fs
           catch2testmain
           config
)
target_include_directories(config_test PUBLIC "${PROJECT_SOURCE_DIR}/src")
