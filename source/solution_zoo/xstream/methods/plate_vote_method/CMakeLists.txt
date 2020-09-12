cmake_minimum_required(VERSION 2.8)

project(plate_vote_method)

include_directories(include)
file(GLOB SOURCE_FILES "src/*.cpp")

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/plate_vote_method.h
        DESTINATION ${MY_OUTPUT_ROOT}/include/${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/lib)

add_subdirectory(test)

