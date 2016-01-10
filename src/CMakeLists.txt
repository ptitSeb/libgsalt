include_directories(qlim)

file(GLOB_RECURSE BASE_SOURCES *.cpp)
file(GLOB_RECURSE QSLIM_SOURCES qslim/*.cpp)

add_library(gsalt SHARED ${BASE_SOURCES} ${QSLIM_SOURCES})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(gsalt m)
endif()

set(GSALT_VERSION_MAJOR 0)
set(GSALT_VERSION_MINOR 1)
set(GSALT_VERSION_STRING ${GSALT_VERSION_MAJOR}.${GSALT_VERSION_MINOR})

set_target_properties(gsalt PROPERTIES VERSION ${GSALT_VERSION_STRING} SOVERSION ${GSALT_VERSION_MAJOR})

INSTALL(FILES ../include/gsalt/gsalt.h DESTINATION include/gsalt)
INSTALL(TARGETS gsalt LIBRARY DESTINATION lib)
