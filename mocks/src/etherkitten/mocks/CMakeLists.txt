find_package(SOEM)

set(SOURCES
    readermock.cpp
    slaveinformantmock.cpp
    esiparsermock.cpp
)

set(HEADERS
    readermock.hpp
    slaveinformantmock.hpp
    esiparsermock.hpp
)

add_library(mocks STATIC ${SOURCES} ${HEADERS})
target_link_libraries(mocks PUBLIC datatypes ${SOEM_LIBRARIES})
target_include_directories(
    mocks
    PUBLIC "${PROJECT_SOURCE_DIR}/../datatypes/src"
    PUBLIC "${PROJECT_SOURCE_DIR}/../reader/src" SYSTEM
    PUBLIC ${SOEM_INCLUDE_DIR}
)
