cmake_minimum_required (VERSION 3.0)
project (circlesim)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "/opt/circlesim" CACHE PATH "default install path" FORCE )
endif()

# The version number.
set (CIRCLESIM_VERSION_MAJOR 1)
set (CIRCLESIM_VERSION_MINOR 0)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/src/version.hpp.in"
  "${PROJECT_BINARY_DIR}/version.hpp"
  )
  
add_subdirectory (src)
  
add_executable(circlesim        src/config.cpp src/main.cpp src/output.cpp src/plot.cpp src/simulator.cpp src/util.cpp src/response.cpp src/labmap.cpp src/script.cpp src/field_map.cpp src/gradient.cpp )
add_executable(circlesim-viewer src/viewer.cpp src/config.cpp src/util.cpp src/gradient.cpp src/frame_controller_base.cpp src/time_controller.cpp)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")


find_package (PNG REQUIRED)
if (PNG_FOUND)
  include_directories(${PNG_INCLUDE_DIRS})
  target_link_libraries (circlesim ${PNG_LIBRARIES})
endif (PNG_FOUND)

find_package(CBLAS REQUIRED)
if (CBLAS_FOUND)
  include_directories(${CBLAS_INCLUDE_DIR})
  target_link_libraries (circlesim ${CBLAS_LIBRARIES})
endif (CBLAS_FOUND)

find_package(GSL REQUIRED) 
if (GSL_FOUND)
  include_directories(${GSL_INCLUDES})
  target_link_libraries (circlesim ${GSL_LIBRARIES})
endif (GSL_FOUND)

find_package(Config REQUIRED) 
if (CONFIG_FOUND)
  include_directories(${CONFIG_INCLUDE_DIR})
  target_link_libraries (circlesim ${CONFIG_LIBRARIES})
  target_link_libraries (circlesim-viewer ${CONFIG_LIBRARIES})
endif (CONFIG_FOUND)

find_package(Check REQUIRED) 
if (CHECK_FOUND)
  include_directories(${CHECK_INCLUDE_DIRS})
  target_link_libraries (circlesim ${CHECK_LIBRARIES})
endif (CHECK_FOUND)


set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

find_package(Boost REQUIRED COMPONENTS filesystem system regex) 
if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries (circlesim ${Boost_LIBRARIES})
  target_link_libraries (circlesim-viewer ${Boost_LIBRARIES})
endif (Boost_FOUND)

find_package(OpenMP REQUIRED) 
if (OPENMP_FOUND)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
endif (OPENMP_FOUND)

find_package(DL REQUIRED) 
if (HAVE_DL)
  include_directories(${DL_INCLUDES})
  target_link_libraries (circlesim ${DL_LIBRARIES})
endif (HAVE_DL)

find_package(Armadillo REQUIRED) 
if (ARMADILLO_FOUND)
  include_directories(${ARMADILLO_INCLUDE_DIRS})
  target_link_libraries (circlesim ${ARMADILLO_LIBRARIES})
  target_link_libraries (circlesim-viewer ${ARMADILLO_LIBRARIES})
endif (ARMADILLO_FOUND)

find_package(Irrlicht REQUIRED) 
if (IRRLICHT_FOUND)
  include_directories(${IRRLICHT_INCLUDE_DIRS})
  target_link_libraries (circlesim-viewer ${IRRLICHT_LIBRARIES})
endif (IRRLICHT_FOUND)


macro(use_cxx11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} --std=c++11")
  else ()
    set (CMAKE_CXX_STANDARD 11)
  endif ()
endmacro(use_cxx11)

use_cxx11()

install(TARGETS circlesim 			RUNTIME DESTINATION bin)
install(TARGETS circlesim-viewer 	RUNTIME DESTINATION bin)

install(DIRECTORY util 				DESTINATION .)


