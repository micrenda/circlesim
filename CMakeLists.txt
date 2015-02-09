cmake_minimum_required(VERSION 2.8)
project(CIRCLESIM CXX)

# Adding path under project folder where I put custom modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

#There are lots of scripts with cmake
#for finding external libraries. 
#see /usr/share/cmake-2.8/Modules/Find*.cmake for more examples

find_package(Boost 		1.55.0 COMPONENTS filesystem system regex	REQUIRED)
find_package(GSL     												REQUIRED)
find_package(LibConfig 												REQUIRED)
find_package(Armadillo 												REQUIRED)
find_package(Lua52     												REQUIRED)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -I${CMAKE_SOURCE_DIR}/util/LuaState/include/")

add_subdirectory(src)
