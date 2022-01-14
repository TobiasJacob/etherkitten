find_package(SOEM)

set(SOURCES
    DatatypesSerializer.cpp
    logger.cpp
    log/coe.cpp
    log/coedata.cpp
    log/CoEUpdate.cpp
    log/coeentry.cpp
    log/DataViewWrapper.cpp
    log/esi.cpp
    log/error.cpp
    log/neighbors.cpp
    log/pdo.cpp
    log/pdodetails.cpp
    log/processdata.cpp
    log/registerdata.cpp
    log/slave.cpp
    log/slavedetails.cpp
    log/Serialized.cpp
    BusQueues.cpp
    CoEUpdateRequest.cpp
    BusSlaveInformant.cpp
    BusSlaveInformant-impl/impl-common.cpp
    BusSlaveInformant-impl/esi.cpp
    BusSlaveInformant-impl/coe.cpp
    BusSlaveInformant-impl/pdo.cpp
    BusSlaveInformant-impl/topology.cpp
    BusSlaveInformant-impl/slavename.cpp
    SearchListReader.cpp
    PDOWriteRequest.cpp
    SlaveInformant.cpp
    LogReader.cpp
    LogSlaveInformant.cpp
    RegisterScheduler.cpp
    EtherCATFrame.cpp
    EtherKitten.cpp
    ErrorStatistician.cpp
    BusReader.cpp
    BusReader-impl/rtfuncs.cpp
    BusReader-impl/busfunctions.cpp
    QueueCacheProxy.cpp
    ReaderErrorIterator.cpp
    LogCache.cpp
)

set(HEADERS
    IOMap.hpp
    LogCache.hpp
    queues-common.hpp
    BusQueues.hpp
    DataView.hpp
    CoEUpdateRequest.hpp
    CoENewestValueView.hpp
    Converter.hpp
    DatatypesSerializer.hpp
    ErrorStatistician.hpp
    endianness.hpp
    EtherCATFrame.hpp
    EtherKitten.hpp
    LLNode.hpp
    logger.hpp
    LogReader.hpp
    LogSlaveInformant.hpp
    log/Block.hpp
    log/CoEUpdate.hpp
    log/DataViewWrapper.hpp
    log/ParsingContext.hpp
    log/Serialized.hpp
    log/Serializer.hpp
    log/coe.hpp
    log/coedata.hpp
    log/coeentry.hpp
    log/esi.hpp
    log/error.hpp
    log/neighbors.hpp
    log/pdo.hpp
    log/pdodetails.hpp
    log/processdata.hpp
    log/registerdata.hpp
    log/slave.hpp
    log/slavedetails.hpp
    MessageQueues.hpp
    NewestValueView.hpp
    PDOWriteRequest.hpp
    QueueCacheProxy.hpp
    Reader.hpp
    SearchListReader.hpp
    RegisterScheduler.hpp
    RingBuffer.hpp
    ErrorRingBuffer.hpp
    SearchList.hpp
    ReaderErrorIterator.hpp
    SlaveInformant.hpp
    BusSlaveInformant.hpp
    BusSlaveInformant-impl/impl-common.hpp
    BusSlaveInformant-impl/esi.hpp
    BusSlaveInformant-impl/coe.hpp
    BusSlaveInformant-impl/pdo.hpp
    BusSlaveInformant-impl/topology.hpp
    BusSlaveInformant-impl/slavename.hpp
    BusReader.hpp
    BusReader-impl/rtfuncs.hpp
    BusReader-impl/busfunctions.hpp
    viewtemplates.hpp
)

add_library(reader STATIC ${SOURCES} ${HEADERS})
set_target_properties(reader PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(
    reader
    PUBLIC datatypes
           ${SOEM_LIBRARIES}
           pthread
)
target_include_directories(
    reader
    PUBLIC "${PROJECT_SOURCE_DIR}/../datatypes/src" SYSTEM
    PUBLIC ${SOEM_INCLUDE_DIR}
)

if(BUILD_TESTING)
    target_link_libraries(reader PUBLIC mocks)
    target_include_directories(
        reader PUBLIC "${PROJECT_SOURCE_DIR}/../mocks/src"
    )
    target_compile_definitions(reader PUBLIC ENABLE_MOCKS)
endif()

if(ENABLE_RT)
    target_compile_definitions(reader PUBLIC ENABLE_RT)
endif()
