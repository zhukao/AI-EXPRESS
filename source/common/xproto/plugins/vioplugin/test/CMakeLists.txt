cmake_minimum_required(VERSION 2.8)

include_directories(
    include
)

set(SOURCE_FILES gtest_main.cc gtest_api.cpp)

set(target_name ${PROJECT_NAME}_test)
add_executable(${target_name} ${SOURCE_FILES})
target_link_libraries(${target_name} ${PROJECT_NAME} gtest)

