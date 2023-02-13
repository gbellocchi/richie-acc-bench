cmake_minimum_required(VERSION 2.8)

project(app_prj) 

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -g -O3")

link_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_APP_ROOT}/include)
include_directories(${CMAKE_APP_UTILS})

file(GLOB driver_0
    "${CMAKE_APP_ROOT}/src/*_orig.c"
)

file(GLOB driver_1
    "${CMAKE_APP_ROOT}/src/*_orig_linux.c"
)

file(GLOB driver_main
    "${CMAKE_APP_ROOT}/src/main.c"
)

add_library(
    app_hw
    ${driver_0} 
)

add_library(
    app_hw_linux
    ${driver_1}
)

add_executable(
    app_exec
    ${driver_main}
)

target_link_libraries(
    app_exec
    app_hw
    app_hw_linux
)