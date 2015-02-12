file(GLOB MyCSources *.cpp)

add_executable(circlesim ${MyCSources})
target_link_libraries(circlesim gsl gslcblas config++ boost_filesystem
boost_system boost_regex armadillo lua5.1)
