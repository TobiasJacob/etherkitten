project(datatypes)
add_subdirectory(src/etherkitten/datatypes)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
