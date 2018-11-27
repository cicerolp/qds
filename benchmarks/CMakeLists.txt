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
        ../source/util/benchmark_util.cpp
        ../source/Temporal.cpp
        ../source/Raw.cpp
        ../source/PDigest.cpp
        ../source/Gaussian.cpp
        ../source/Pipeline.cpp
        ../source/AugmentedSeries.cpp
        ../source/Clustering.cpp
        ../source/PostGisCtn.cpp
        ../source/SpatiaLiteCtn.cpp
        ../source/MonetDBCtn.cpp)

add_executable(bench_queries ${HEADERS} ${SRC_LIST} bench_queries.cpp)

target_link_libraries(bench_queries LINK_PUBLIC ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

add_executable(bench_info ${HEADERS} ${SRC_LIST} bench_info.cpp)

target_link_libraries(bench_info LINK_PUBLIC ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})


FIND_LIBRARY(PQ_C_LIBRARY NAMES pq)
FIND_LIBRARY(GEOS_C_LIBRARY NAMES geos_c)
FIND_LIBRARY(SPATIALITE_LIBRARY NAMES spatialite)
FIND_LIBRARY(SQLITE_LIBRARY NAMES sqlite3)

add_executable(bench_queries_db ${HEADERS} ${SRC_LIST} bench_queries_db.cpp)

target_link_libraries(bench_queries_db LINK_PUBLIC ${CMAKE_DL_LIBS}
        ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES}
        ${SPATIALITE_LIBRARY} ${SQLITE_LIBRARY} ${GEOS_C_LIBRARY} ${PQ_C_LIBRARY})

# # gperftools
# find_package(Gperftools REQUIRED)
# include_directories(GPERFTOOLS_INCLUDE_DIR)

# # profiler
# add_executable(profiler ${HEADERS} ${SRC_LIST} profiler.cpp)
# target_link_libraries(profiler LINK_PUBLIC ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${COMMON_LIBRARIES} ${GPERFTOOLS_PROFILER})
