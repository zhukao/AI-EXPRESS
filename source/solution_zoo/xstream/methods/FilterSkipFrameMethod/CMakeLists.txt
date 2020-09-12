cmake_minimum_required(VERSION 2.8)

project(FilterSkipFrameMethod)

include_directories(
        ${CMAKE_CURRENT_LIST_DIR}/include
)

set(SOURCE_FILES
  ${CMAKE_CURRENT_LIST_DIR}/src/method/filter_skip_frame.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/include)
install(TARGETS ${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/lib)

add_subdirectory(test)

