cmake_minimum_required(VERSION 3.8)

project(prj_aes_128_sw C CXX)

set(CMAKE_SYSROOT ${SYSROOT})

if(DEFINED ENV{HERO_INSTALL})
    set(CMAKE_C_COMPILER aarch64-hero-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER aarch64-hero-linux-gnu-gcc)
else()
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER aarch64-linux-gnu-gcc)
endif()

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_BINARY_DIR "${CMAKE_APP_ROOT}/build")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3 -std=c++11")

set(CMAKE_CXX_STANDARD 14)

link_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_APP_UTILS})
include_directories(${CMAKE_APP_ROOT}/src/inc)

set(${CMAKE_APP_ROOT}/src/main.cpp)

ADD_LIBRARY(
    LibsModule
    ${CMAKE_APP_ROOT}/src/inc/aes_sw.h
    ${CMAKE_APP_ROOT}/src/inc/LCM.h 
    ${CMAKE_APP_ROOT}/src/inc/Taks.h 
    ${CMAKE_APP_ROOT}/src/inc/aes.h 
    ${CMAKE_APP_ROOT}/src/inc/aes256.h 
    ${CMAKE_APP_ROOT}/src/inc/aes256.c
)

add_executable(
    ${CMAKE_APP_NAME}
    ${CMAKE_APP_ROOT}/src/main.cpp
)

target_link_libraries(
    ${CMAKE_APP_NAME}
    LibsModule -lpthread
)