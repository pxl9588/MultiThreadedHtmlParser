cmake_minimum_required(VERSION 3.20)

project(MultiThreaded_HTML_Parser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

add_definitions(-DCURL_STATICLIB)
include_directories(${CMAKE_SOURCE_DIR}/include)
#link_directories(${CMAKE_SOURCE_DIR}/lib)

set(SOURCES src/main.cpp src/URLParser.cpp include/URLParser.h)

add_library(tidy STATIC IMPORTED) # or STATIC instead of SHARED
set_target_properties(tidy PROPERTIES
  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/libtidys.a"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/include/tidy"
)

add_executable(MultiThreaded_HTML_Parser src/main.cpp src/URLParser.cpp)
target_link_libraries(MultiThreaded_HTML_Parser curl tidy)