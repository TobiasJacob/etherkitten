find_package(Catch2 REQUIRED)
add_library(catch2testmain STATIC test_main.cpp test_globals.hpp)
target_link_libraries(catch2testmain PUBLIC Catch2::Catch2)
