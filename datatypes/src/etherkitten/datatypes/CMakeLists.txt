set(SOURCES
    time.cpp
    datapoints.cpp
    SlaveInfo.cpp
    errors.cpp
    esiparser.cpp
    ethercatdatatypes.cpp
    coe.cpp
    pdo.cpp
    errorstatistic.cpp
    register.cpp
    DataObject.cpp
)

set(HEADERS
    SlaveInfo.hpp
    esiparser.hpp
    ethercatdatatypes.hpp
    dataobjects.hpp
    DataObjectVisitor.hpp
    esidata.hpp
    datapoints.hpp
    dataviews.hpp
    time.hpp
    errors.hpp
    EtherCATTypeStringFormatter.hpp
    EtherCATTypeParser.hpp
    coe.hpp
    pdo.hpp
    errorstatistic.hpp
    register.hpp
    DataObject.hpp
)

add_library(datatypes STATIC ${SOURCES} ${HEADERS})

set_target_properties(datatypes PROPERTIES LINKER_LANGUAGE CXX)
