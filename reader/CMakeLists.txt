project(reader)
add_subdirectory(src/etherkitten/reader)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
