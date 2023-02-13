cmake_minimum_required(VERSION 2.8)

project(mmult_arm) 

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -g -O3")

link_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_APP_UTILS})

add_executable(
    ${CMAKE_APP_NAME}
    ${CMAKE_APP_ROOT}/src/main.c
)

target_link_libraries(
    ${CMAKE_APP_NAME}
)