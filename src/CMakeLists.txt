include_directories(qlim)

file(GLOB_RECURSE BASE_SOURCES *.c)
file(GLOB_RECURSE QSLIM_SOURCES qslim/*.cpp)

add_library(gsalt SHARED ${BASE_SOURCES} ${QSLIM_SOURCES})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(gsalt m)
endif()

if(CMAKE_SHARED_LIBRARY_SUFFIX MATCHES ".so")
    set_target_properties(gsalt PROPERTIES SUFFIX ".so.0")
endif()

