project (nds)
cmake_minimum_required (VERSION 2.8.11)

set(BOOST_INCLUDEDIR "C:/Boost/include/boost-1_62/")
set(BOOST_LIBRARYDIR "C:/Boost/lib/")

add_custom_target(debug
  COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
  COMMENT "Switch CMAKE_BUILD_TYPE to Debug"
)

add_custom_target(release
  COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
  COMMENT "Switch CMAKE_BUILD_TYPE to Release"
)

add_custom_target(clang
  COMMAND ${CMAKE_COMMAND} -DCMAKE_C_COMPILER=/usr/bin/clang  -DCMAKE_CXX_COMPILER=/usr/bin/clang++ ${CMAKE_SOURCE_DIR}
  COMMENT "Switch CMAKE_C/CXX_COMPILER to Clang"
)

add_custom_target(gcc
  COMMAND ${CMAKE_COMMAND} -DCMAKE_C_COMPILER=/usr/bin/gcc -DCMAKE_CXX_COMPILER=/usr/bin/g++ ${CMAKE_SOURCE_DIR}
  COMMENT "Switch CMAKE_C/CXX_COMPILER to GCC"
)

if(MSVC)
   set(CMAKE_CXX_FLAGS "/EHsc")
endif()

if(UNIX)
   set(CMAKE_CXX_FLAGS "-std=c++1z")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

include(cotire)

aux_source_directory(. SRC_LIST)

include_directories(AFTER include)
include_directories(AFTER source)
include_directories(AFTER ./)

add_executable (${PROJECT_NAME} ${SRC_LIST}
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
)

find_package(Threads REQUIRED)

set(Boost_USE_MULTITHREAD ON)
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost 1.62 COMPONENTS system filesystem serialization iostreams REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

target_link_libraries (${PROJECT_NAME} LINK_PUBLIC ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(${PROJECT_NAME} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "source/stdafx.h")

cotire(${PROJECT_NAME})
