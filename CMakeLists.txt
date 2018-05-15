project(nds)
cmake_minimum_required(VERSION 3.0)

set (CMAKE_CXX_STANDARD 17)

option(ENABLE_GPERF "Enable Gperftools Profiler" OFF)

# nds options
option(NDS_ENABLE_PAYLOAD "Enable Payload" ON)

# nds tweaks
option(NDS_OPTIMIZE_LEAF "Enable Spatial Element Leaf Optimization" OFF)

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

configure_file(source/config.h.in config.h)
include_directories("${CMAKE_CURRENT_BINARY_DIR}") # To find the .h

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

include(cotire)

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
        source/string_util.cpp
        source/Temporal.cpp
        source/PDigest.cpp
        source/Gaussian.cpp
        source/Pipeline.cpp
        source/AugmentedSeries.cpp
        source/Clustering.cpp)

add_executable(${PROJECT_NAME} ${SRC_LIST} ${HEADERS})

find_package(Threads REQUIRED)

set(Boost_USE_MULTITHREAD ON)
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost COMPONENTS program_options REQUIRED)

link_directories(${Boost_LIBRARY_DIR})
include_directories(${Boost_INCLUDE_DIR})


if (WIN32)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
elseif (UNIX)
    set(COMMON_LIBRARIES "-static-libgcc -static-libstdc++ ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}")

    # default target
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${COMMON_LIBRARIES})

    # profiling target
    find_package(Gperftools REQUIRED)
    include_directories(GPERFTOOLS_INCLUDE_DIR)
    add_executable(profile ${SRC_LIST} ${HEADERS})
    target_link_libraries(profile LINK_PUBLIC ${COMMON_LIBRARIES} ${GPERFTOOLS_PROFILER})
    set_target_properties(profile PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "source/stdafx.h")
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "source/stdafx.h")

cotire(${PROJECT_NAME})
