project(controller)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_subdirectory(src/etherkitten/controller)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
