file(GLOB HEADERS "*.h")
source_group("Header Files" FILES ${HEADERS})

set(SRC_LIST
        ../source/mongoose.c
        ../source/Categorical.cpp
        ../source/Data.cpp
        ../source/NDS.cpp
        ../source/NDSInstances.cpp
        ../source/Query.cpp
        ../source/Server.cpp
        ../source/Spatial.cpp
        ../source/SpatialElement.cpp
        ../source/util/string_util.cpp
        ../source/Temporal.cpp
        ../source/Raw.cpp
        ../source/PDigest.cpp
        ../source/Gaussian.cpp
        ../source/Pipeline.cpp
        ../source/AugmentedSeries.cpp)

add_executable(bench_queries ${HEADERS} ${SRC_LIST}
        bench_queries.cpp)

target_link_libraries(bench_queries LINK_PUBLIC ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})