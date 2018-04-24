cmake_minimum_required(VERSION 3.5)

project(GROUP9.IMU)

set(CMAKE_CXX_STANDARD 14)

find_package(libcluon REQUIRED)

include_directories(SYSTEM ${CLUON_INCLUDE_DIRS})
include_directories(includes/Eigen)
include_directories(includes)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/Messages.cpp
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND cluon-msc --cpp-sources --cpp-add-include-file=${CMAKE_BINARY_DIR}/Messages.hpp --out=${CMAKE_BINARY_DIR}/Messages.cpp ${CMAKE_CURRENT_SOURCE_DIR}/includes/opendlv-standard-message-set-v0.9.4.odvd
        COMMAND cluon-msc --cpp-headers --out=${CMAKE_BINARY_DIR}/Messages.hpp ${CMAKE_CURRENT_SOURCE_DIR}/includes/opendlv-standard-message-set-v0.9.4.odvd
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/includes/opendlv-standard-message-set-v0.9.4.odvd)

include_directories(SYSTEM ${CMAKE_BINARY_DIR})

add_executable(${PROJECT_NAME} IMU.cpp ${CMAKE_BINARY_DIR}/Messages.cpp includes/IMU.hpp)
target_link_libraries(${PROJECT_NAME} ${CLUON_LIBRARIES})
