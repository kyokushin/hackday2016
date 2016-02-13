cmake_minimum_required(VERSION 3.0)

find_package(OpenCV 3.0 REQUIRED)

PROJECT(OpenHackDay4)

set(IMAGE_FILE_PATH_0 "${CMAKE_CURRENT_SOURCE_DIR}/data/DSC_0059.JPG")
set(IMAGE_FILE_PATH_1 "${CMAKE_CURRENT_SOURCE_DIR}/data/DSC_0060.JPG")
set(IMAGE_FILE_PATH_2 "${CMAKE_CURRENT_SOURCE_DIR}/data/DSC_0061.JPG")

set(INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")
set(CONFIG_FILE "${INCLUDE_DIR}/configure.h")


add_definitions(${OpenCV_DEFINITIONS})
include_directories(${OpenCV_INCLUDE_DIRS} ${INCLUDE_DIR})
link_directories(${OpenCV_LIBRARY_DIRS})

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/configure.h.in" ${CONFIG_FILE} @ONLY)

add_executable(main main.cpp ${CONFIG_FILE})
target_link_libraries(main ${OpenCV_LIBRARIES})

set(CompilerFlags
    CMAKE_CXX_FLAGS
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_C_FLAGS
    CMAKE_C_FLAGS_DEBUG
    CMAKE_C_FLAGS_RELEASE)
foreach(CompilerFlag ${CompilerFlags})
    string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()
