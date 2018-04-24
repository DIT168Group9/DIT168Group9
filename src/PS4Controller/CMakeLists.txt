cmake_minimum_required(VERSION 3.5)

project(GROUP9.PS4Controller)

set(CMAKE_CXX_STANDARD 14)

find_package(libcluon REQUIRED)

include_directories(SYSTEM ${CLUON_INCLUDE_DIRS})
include_directories(includes)

include_directories(SYSTEM ${CMAKE_BINARY_DIR})

add_executable(${PROJECT_NAME} PS4Controller.cpp ${CMAKE_BINARY_DIR}/Messages.cpp PS4Controller.cpp includes/PS4Controller.hpp)
target_link_libraries(${PROJECT_NAME} ${CLUON_LIBRARIES})
