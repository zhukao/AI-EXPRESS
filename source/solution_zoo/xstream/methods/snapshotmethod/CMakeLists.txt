cmake_minimum_required(VERSION 2.8)

project(SnapShotMethod)

option(RELEASE_LIB "build version of release" ON)
if (${RELEASE_LIB})
    set(CMAKE_BUILD_TYPE Release)
else ()
    set(CMAKE_BUILD_TYPE Debug)
endif ()

set(CMAKE_CXX_STANDARD 11)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -fPIC -O3")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DEBUG -g -O0 -fPIC")
endif()

include_directories(
        include
)

set(BASE_LIBRARIES xstream-imagetools libturbojpeg.a libyuv.a jsoncpp.a hobotlog rt dl pthread)

set(SOURCE_FILES
        ${CMAKE_CURRENT_LIST_DIR}/src/method/SnapShotMethod.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/method/strategy/crop.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/method/strategy/first_num_best.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/method/image_utils/image_utils.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/method/snapshot_data_type/snapshot_data_type.cpp
)

link_directories(/usr/local/cuda/lib64/)
add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${BASE_LIBRARIES})

if(NOT ${PARENT_BUILD})
set(OUTPUT_ROOT ${CMAKE_SOURCE_DIR}/output/)
endif()

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)

install(FILES include/SnapShotMethod/SnapShotMethod.h
        DESTINATION ${MY_OUTPUT_ROOT}/include/${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/lib)

add_subdirectory(test)
