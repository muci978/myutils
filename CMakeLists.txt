cmake_minimum_required(VERSION 3.21)
project(myutils)
set(CMAKE_CXX_STANDARD 11)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/3rd/include
    ${CMAKE_SOURCE_DIR}/core)

file(GLOB SRC_LIST ${CMAKE_SOURCE_DIR}/core/*.cpp)

link_directories(${CMAKE_SOURCE_DIR}/lib)

link_libraries(yaml-cpp)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
add_executable(${PROJECT_NAME} ${SRC_LIST})
