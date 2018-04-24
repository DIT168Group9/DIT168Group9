cmake_minimum_required(VERSION 3.5)

project(GROUP9.IMU)

set(CMAKE_CXX_STANDARD 14)

find_package(libcluon REQUIRED)

include_directories(SYSTEM ${CLUON_INCLUDE_DIRS})
include_directories(includes/Eigen)
include_directories(includes)

include_directories(SYSTEM ${CMAKE_BINARY_DIR})

add_executable(${PROJECT_NAME} IMU.cpp ${CMAKE_BINARY_DIR}/Messages.cpp includes/IMU.hpp)
target_link_libraries(${PROJECT_NAME} ${CLUON_LIBRARIES})
