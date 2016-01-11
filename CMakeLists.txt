cmake_minimum_required(VERSION 2.6)

project(gsalt)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(EXAMPLES "Compile Examples (you will need GL, GLU and glut)" ${EXAMPLES})

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

link_directories(${CMAKE_BINARY_DIR}/lib)

include_directories(include)
add_subdirectory(src)

if(EXAMPLES)
 add_subdirectory(examples) 
endif()
