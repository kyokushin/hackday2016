cmake_minimum_required(VERSION 3.0)

find_package(OpenCV 3.1 REQUIRED EXACT)

PROJECT(HackDay2016)

if(UNIX)
	set(PATH_SEPARATOR "/")
else()
	set(PATH_SEPARATOR "\\\\")
endif()


set(INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")
set(CONFIG_FILE "${INCLUDE_DIR}/configure.h")


add_definitions(${OpenCV_DEFINITIONS})
include_directories(${OpenCV_INCLUDE_DIRS} ${INCLUDE_DIR})
link_directories(${OpenCV_LIBRARY_DIRS})

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(main ${SOURCES} ${CONFIG_FILE})
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
