cmake_minimum_required(VERSION 3.10)
project(LESI_VC_2324_2)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} include)

add_executable(main src/main.cpp src/vc.c
        src/video_processor.cpp
        src/utility.cpp
        src/resistor_detection.cpp
        src/image_processing.cpp
        include/video_processor.h
        include/resistor_detection.h
        include/image_processing.h
        include/utility.h)

target_link_libraries(main ${OpenCV_LIBS})
