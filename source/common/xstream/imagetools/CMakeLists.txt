cmake_minimum_required(VERSION 2.8)

project(xstream-imagetools)

option(BUILD_X2_LIB "build x2 lib" ON)

if(BGR2I420_WITH_LIBYUV)
        add_definitions(-DBGR2I420_WITH_LIBYUV)
endif(BGR2I420_WITH_LIBYUV)

include_directories(
        include
)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} libturbojpeg.a libyuv.a opencv_world)

add_subdirectory(test)
add_subdirectory(example)

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/hobotxstream
        DESTINATION ${MY_OUTPUT_ROOT}/include)
install(TARGETS ${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/lib)

