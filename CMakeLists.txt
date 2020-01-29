cmake_minimum_required(VERSION 2.8)
PROJECT(thermaltracker)
find_package(OpenCV REQUIRED)
set(TRACKER_SRC src/thermaltracker.cpp)
set(TRACKER_HEADERS include/thermaltracker.h)

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
link_directories(${CMAKE_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
add_executable(thermaltracker ${TRACKER_SRC} ${TRACKER_HEADERS})

target_link_libraries(thermaltracker ${OpenCV_LIBS})
