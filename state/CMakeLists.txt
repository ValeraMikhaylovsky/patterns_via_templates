cmake_minimum_required(VERSION 3.14)

project(state LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(state meta_fsm.h main.cpp)
