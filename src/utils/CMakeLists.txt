cmake_minimum_required(VERSION 3.19)
project(ComputeLUT)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(/opt/homebrew/include)

add_executable(ComputeLUT LUTCompute.cpp)
target_link_libraries(ComputeLUT ${OpenCV_LIBS})