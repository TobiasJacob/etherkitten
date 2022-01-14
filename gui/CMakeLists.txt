project(gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_subdirectory(src/etherkitten/gui)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
