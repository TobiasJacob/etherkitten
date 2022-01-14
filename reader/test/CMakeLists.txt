find_package(Catch2 REQUIRED)

set(HEADERS
    DataReaderMock.hpp
    SlaveInformantMock.hpp
    ThreadContainer.hpp
)

set(SOURCES
    BusQueuesTest.cpp
    DataViewTest.cpp
    NewestValueViewtest.cpp
    CoENewestValueViewtest.cpp
    LLNodetest.cpp
    SearchListTest.cpp
    DataReaderMock.cpp
    SlaveInformantMock.cpp
    basiclogtests.cpp
    BusSlaveInformanttest.cpp
    RegisterSchedulertest.cpp
    ErrorStatisticiantest.cpp
    serializationtest.cpp
    BusReadertest.cpp
    QueueCacheProxytest.cpp
    Convertertest.cpp
    PDOWriteRequestTest.cpp
    ErrorRingBufferTest.cpp
    RingBufferTest.cpp
    CoEUpdateRequestest.cpp
    TripleBuffertest.cpp
    viewtemplatestest.cpp
    LogCacheTest.cpp
)

add_executable(reader_test ${SOURCES} ${HEADERS})
target_link_libraries(
    reader_test
    PUBLIC atomic
           stdc++fs
           catch2testmain
           reader
)

target_include_directories(
    reader_test PUBLIC "${PROJECT_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/../"
)
