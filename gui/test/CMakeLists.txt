find_package(Qt5 COMPONENTS Widgets)

add_executable(profile_open_dialog_test "ProfileOpenDialogTest.cpp")
target_link_libraries(profile_open_dialog_test PRIVATE gui)
target_include_directories(
    profile_open_dialog_test PUBLIC "${PROJECT_SOURCE_DIR}/src"
)

add_executable(profile_save_dialog_test "ProfileSaveDialogTest.cpp")
target_link_libraries(profile_save_dialog_test PRIVATE gui)
target_include_directories(
    profile_save_dialog_test PUBLIC "${PROJECT_SOURCE_DIR}/src"
)

add_executable(log_start_dialog_test "LogStartDialogTest.cpp")
target_link_libraries(log_start_dialog_test PRIVATE gui)
target_include_directories(
    log_start_dialog_test PUBLIC "${PROJECT_SOURCE_DIR}/src"
)

add_executable(plot_test "PlotTest.cpp" "PlotTest.hpp")
target_link_libraries(plot_test PRIVATE gui datatypes)
target_include_directories(plot_test PUBLIC "${PROJECT_SOURCE_DIR}/src")

add_executable(slave_tree_test "SlaveTreeTest.cpp" "SlaveTreeTest.hpp")
target_link_libraries(slave_tree_test PRIVATE gui datatypes)
target_include_directories(slave_tree_test PUBLIC "${PROJECT_SOURCE_DIR}/src")

add_executable(register_chooser_test "RegisterChooserTest.cpp")
target_link_libraries(register_chooser_test PRIVATE gui datatypes)
target_include_directories(
    register_chooser_test PUBLIC "${PROJECT_SOURCE_DIR}/src"
)

add_executable(interface_chooser_test "InterfaceChooserTest.cpp")
target_link_libraries(interface_chooser_test PRIVATE gui)
target_include_directories(
    interface_chooser_test PUBLIC "${PROJECT_SOURCE_DIR}/src"
)

add_executable(slave_graph_test "SlaveGraphTest.cpp" "SlaveGraphTest.hpp")
target_link_libraries(slave_graph_test PRIVATE gui datatypes)
target_include_directories(slave_graph_test PUBLIC "${PROJECT_SOURCE_DIR}/src")
