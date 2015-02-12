cmake_minimum_required(VERSION 2.8.0)
project(CIRCLESIM CXX)

# Adding path under project folder where I put custom modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

#There are lots of scripts with cmake
#for finding external libraries. 
#see /usr/share/cmake-2.8/Modules/Find*.cmake for more examples

find_package(Boost      1.41.0 COMPONENTS filesystem system regex   REQUIRED)
find_package(GSL                                                    REQUIRED)
find_package(LibConfig                                              REQUIRED)
find_package(Armadillo                                              REQUIRED)
find_package(Lua51                                                  REQUIRED)
find_package(OpenMP)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -I${CMAKE_SOURCE_DIR}/util/LuaState/include/ -I//usr/include/lua5.1/")

if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

add_subdirectory(src)
