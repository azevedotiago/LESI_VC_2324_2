cmake_minimum_required(VERSION 3.10)
project(LESI_VC_2324_2)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(main src/main.cpp include/vc.c)

target_link_libraries(main ${OpenCV_LIBS})
