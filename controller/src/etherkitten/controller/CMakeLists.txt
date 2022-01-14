find_package(
    Qt5
    COMPONENTS Core LinguistTools
    REQUIRED
)

set(SOURCES
    Application.cpp
    EtherKittenBusInfoSupplier.cpp
    EtherKittenDataModelAdapter.cpp
    visitors.cpp
    InterfaceInfo.cpp
)

set(HEADERS
    Application.hpp
    EtherKittenBusInfoSupplier.hpp
    EtherKittenDataModelAdapter.hpp
    visitors.hpp
    InterfaceInfo.hpp
)

# IMPORTANT: If cmake has already been run before etherkitten_en_secret.ts is
# added, you need to force it to run again since this test for the existence
# of the file is not run on every `make`.

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/i18n/etherkitten_en_secret.ts")
    set(translationFiles i18n/etherkitten_en.ts i18n/etherkitten_en_secret.ts)
else()
    set(translationFiles i18n/etherkitten_en.ts)
endif()

set(qmResourcesFile i18n.qrc)

# See
# https://stackoverflow.com/questions/51217734/how-to-add-qt-translations-to-cmake
set(qmDir "${CMAKE_CURRENT_BINARY_DIR}/qm")
file(MAKE_DIRECTORY ${qmDir})

set_source_files_properties(
    ${translationFiles} PROPERTIES OUTPUT_LOCATION ${qmDir}
)

qt5_add_translation(
    qmFileList
    ${translationFiles}
)

set(qmResourcesFile "${qmDir}/${qmResourcesFile}")
file(WRITE ${qmResourcesFile}
     "<!DOCTYPE RCC><RCC version=\"1.0\">\n  <qresource prefix=\"i18n\">\n"
)

foreach(qm_file ${qmFileList})
    file(
        RELATIVE_PATH
        qm_filename
        ${qmDir}
        ${qm_file}
    )
    file(APPEND ${qmResourcesFile}
         "    <file alias=\"${qm_filename}\">${qm_file}</file>\n"
    )
endforeach(qm_file)

file(APPEND ${qmResourcesFile} "  </qresource>\n</RCC>\n")

qt5_add_resources(RESOURCES ${qmResourcesFile})

add_executable(
    etherkitten
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
    ${qmFileList}
)

set_target_properties(etherkitten PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(
    etherkitten
    PUBLIC stdc++fs
           Qt5::Core
           datatypes
           config
           reader
           gui
)
target_include_directories(
    etherkitten
    PUBLIC "${PROJECT_SOURCE_DIR}/../datatypes/src"
           "${PROJECT_SOURCE_DIR}/../config/src"
           "${PROJECT_SOURCE_DIR}/../reader/src"
           "${PROJECT_SOURCE_DIR}/../gui/src"
)
