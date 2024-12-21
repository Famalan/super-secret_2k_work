cmake_minimum_required(VERSION 3.10)
project(your_project)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(your_program lab1_v10.cpp)
target_link_libraries(your_program sfml-graphics sfml-window sfml-system)