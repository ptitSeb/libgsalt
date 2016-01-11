cmake_minimum_required(VERSION 2.6)

project(SimpleViewer)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

link_directories(${CMAKE_BINARY_DIR}/bin)

include_directories(include)

include_directories(MeshLib/core)

file(GLOB_RECURSE BASE_SOURCES Viewer/*.cpp)
file(GLOB_RECURSE MESHLIB_SOURCES MeshLib/core/bmp/*.cpp)

add_executable(SimpleViewer ${BASE_SOURCES} ${MESHLIB_SOURCES})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(SimpleViewer m GL GLU glut gsalt)
endif()

