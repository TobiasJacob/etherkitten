set(SOURCES
    BusConfig.cpp
    BusLayout.cpp
    ConfigIO.cpp
    Position.cpp
)

set(HEADERS
    BusConfig.hpp
    BusLayout.hpp
    Position.hpp
    ConfigObserver.hpp
    ConfigIO.hpp
    json.hpp
)

add_library(config STATIC ${SOURCES} ${HEADERS})

set_target_properties(config PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(config PUBLIC stdc++fs)
