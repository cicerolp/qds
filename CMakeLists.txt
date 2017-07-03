project (nds)
cmake_minimum_required (VERSION 3.0)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

if(WIN32)
   if (MSVC)
      set(CMAKE_CXX_FLAGS "/EHsc")
   endif()   

   set(BOOST_LIBRARYDIR "C:/Boost/lib/")
   set(BOOST_INCLUDEDIR "C:/Boost/include/boost-1_62/")
endif()

if(UNIX)
   #set(CMAKE_CXX_FLAGS "-ltcmalloc -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free") 
   find_package(Gperftools REQUIRED)
endif()

include(cotire)

aux_source_directory(. SRC_LIST)

include_directories(AFTER ./)
include_directories(AFTER source)
include_directories(AFTER include)

file(GLOB_RECURSE HEADERS "source/*.h")
source_group("Header Files" FILES ${HEADERS})

add_executable (${PROJECT_NAME} ${SRC_LIST} ${HEADERS}
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

find_package(Boost COMPONENTS program_options REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

target_link_libraries (${PROJECT_NAME} LINK_PUBLIC ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${GPERFTOOLS_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "source/stdafx.h")

cotire(${PROJECT_NAME})
