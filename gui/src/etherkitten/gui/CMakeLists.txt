set(EXE_Name bla)
find_package(
    Qt5
    COMPONENTS Widgets
               Core
               PrintSupport
    REQUIRED
)

set(HEADERS
    qcustomplot.hpp
    TimeStampConverter.hpp
    BusInfoSupplier.hpp
    DataModelAdapter.hpp
    Edge.hpp
    ErrorView.hpp
    GUIController.hpp
    InterfaceChooser.hpp
    LogStartDialog.hpp
    MainWindow.hpp
    Node.hpp
    Plot.hpp
    PlotList.hpp
    RegisterChooser.hpp
    SlaveGraph.hpp
    SlaveTree.hpp
    TooltipFormatter.hpp
    DataEntry.hpp
    WatchList.hpp
    WatchlistVisitor.hpp
    esi.hpp
    util.hpp
    ProfileSaveDialog.hpp
    ProfileOpenDialog.hpp
)

set(SOURCES
    qcustomplot.cpp
    TimeStampConverter.cpp
    GUIController.cpp
    ErrorView.cpp
    WatchList.cpp
    DataEntry.cpp
    WatchlistVisitor.cpp
    SlaveTree.cpp
    MainWindow.cpp
    esi.cpp
    util.cpp
    TooltipFormatter.cpp
    ProfileSaveDialog.cpp
    ProfileOpenDialog.cpp
    InterfaceChooser.cpp
    LogStartDialog.cpp
    RegisterChooser.cpp
    SlaveGraph.cpp
    Node.cpp
    Edge.cpp
    Plot.cpp
    PlotList.cpp
)

set(UIS mainwindow.ui)

set(RESOURCES images.qrc)

add_library(
    gui STATIC
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

set_target_properties(gui PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(
    gui
    PUBLIC config
           datatypes
           Qt5::Widgets
           Qt5::Core
           Qt5::PrintSupport
)
target_include_directories(
    gui PUBLIC "${PROJECT_SOURCE_DIR}/../datatypes/src"
               "${PROJECT_SOURCE_DIR}/../config/src"
)
