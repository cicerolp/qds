project(nds)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 14)

# profile options
option(ENABLE_GPERF "Enable Gperftools Profiler" OFF)
option(ENABLE_METRICS "Enable Metrics" OFF)

# nds options
option(NDS_ENABLE_PAYLOAD "Enable Payload" ON)

option(NDS_ENABLE_CRS_SIMPLE "Enable CRS.Simple" OFF)

# nds tweaks
option(NDS_OPTIMIZE_LEAF "Enable Spatial Element Leaf Optimization" ON)

# nds sharing
option(NDS_SHARE_PIVOT "Enable Pivot Sharing" ON)
option(NDS_SHARE_PAYLOAD "Enable Payload Sharing" ON)

# p-digest
option(ENABLE_PDIGEST "Enable p-digest" ON)

option(PDIGEST_OPTIMIZE_ARRAY "Enable p-digest array optimization" ON)
option(PDIGEST_PIECE_WISE_APPROXIMATION "Enable p-digest Piece Wise Approximation" ON)
set(PDIGEST_COMPRESSION "50.f" CACHE STRING "Set p-digest Compression (float)")
set(PDIGEST_BUFFER_FACTOR "5" CACHE STRING "Set p-digest Buffer Size (N * PDIGEST_ARRAY_SIZE)")

# gaussian
option(ENABLE_GAUSSIAN "Enable Gaussian" ON)

# raw
option(ENABLE_RAW "Enable Raw" OFF)

configure_file(source/config.h.in config.h)
include_directories("${CMAKE_CURRENT_BINARY_DIR}") # To find the .h

aux_source_directory(. SRC_LIST)

include_directories(AFTER ./)
include_directories(AFTER source)
include_directories(AFTER include)

file(GLOB HEADERS "source/*.h")
source_group("Header Files" FILES ${HEADERS})

set(SRC_LIST
        source/mongoose.c
        source/Categorical.cpp
        source/Data.cpp
        source/main.cpp
        source/NDS.cpp
        source/NDSInstances.cpp
        source/Query.cpp
        source/Server.cpp
        source/Spatial.cpp
        source/SpatialElement.cpp
        source/util/string_util.cpp
        source/util/benchmark_util.cpp
        source/Temporal.cpp
        source/Raw.cpp
        source/PDigest.cpp
        source/Gaussian.cpp
        source/Pipeline.cpp
        source/AugmentedSeries.cpp
        source/Clustering.cpp
        source/PostGisCtn.cpp
        source/SpatiaLiteCtn.cpp
        source/MonetDBCtn.cpp)

FIND_LIBRARY(PQ_C_LIBRARY NAMES pq)
FIND_LIBRARY(GEOS_C_LIBRARY NAMES geos_c)
FIND_LIBRARY(SPATIALITE_LIBRARY NAMES spatialite)
FIND_LIBRARY(SQLITE_LIBRARY NAMES sqlite3)

find_package(Threads REQUIRED)
find_package(Boost COMPONENTS serialization program_options regex filesystem REQUIRED)

if(Boost_FOUND)
    set(CMAKE_CXX_STANDARD_LIBRARIES "-lmapi -lssl -lcrypto -lstream -lcurl -llz4 -llzma -lbz2 -lsnappy")

    set(COMMON_LIBRARIES "${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES}")
    # string(STRIP ${COMMON_LIBRARIES} COMMON_LIBRARIES)

    include_directories(${Boost_INCLUDE_DIRS})

    add_executable(${PROJECT_NAME} ${SRC_LIST} ${HEADERS})

    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${COMMON_LIBRARIES}
            ${SPATIALITE_LIBRARY} ${SQLITE_LIBRARY} ${GEOS_C_LIBRARY} ${PQ_C_LIBRARY})
endif()

# # cotire
# set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
# include(cotire)
# set_target_properties(${PROJECT_NAME} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "source/stdafx.h")
# cotire(${PROJECT_NAME})

# # benchmarks
add_subdirectory(benchmarks)
