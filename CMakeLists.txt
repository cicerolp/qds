project(nds)
cmake_minimum_required(VERSION 3.0)

# nds options
option(NDS_ENABLE_PAYLOAD "Enable Payload" OFF)

# nds tweaks
option(NDS_OPTIMIZE_LEAF "Enable Spatial Element Leaf Optimization" ON)

# nds sharing
option(NDS_SHARE_PIVOT "Enable Pivot Sharing" ON)
option(NDS_SHARE_PAYLOAD "Enable Payload Sharing" ON)

# p-digest
option(ENABLE_PDIGEST "Enable p-digest" ON)
option(PDIGEST_PIECE_WISE_APPROXIMATION "Enable p-digest Piece Wise Approximation" ON)
set(PDIGEST_COMPRESSION "50.f" CACHE STRING "Set p-digest Compression (float)")

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

add_executable(${PROJECT_NAME} ${SRC_LIST} ${HEADERS}
        source/mongoose.c
        source/Categorical.cpp
        source/Data.cpp
        source/Dimension.cpp
        source/main.cpp
        source/NDS.cpp
        source/NDSInstances.cpp
        source/Query.cpp
        source/Server.cpp
        source/Spatial.cpp
        source/SpatialElement.cpp
        source/string_util.cpp
        source/Temporal.cpp
        source/PDigest.cpp)

find_package(Threads REQUIRED)

set(Boost_USE_MULTITHREAD ON)
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost COMPONENTS program_options REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

if(WIN32)
        target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
elseif(UNIX)
        target_link_libraries(${PROJECT_NAME} LINK_PUBLIC -static-libgcc -static-libstdc++ ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "source/stdafx.h")

cotire(${PROJECT_NAME})
