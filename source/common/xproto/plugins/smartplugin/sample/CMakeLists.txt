cmake_minimum_required(VERSION 2.8)

include_directories(
    include
)

set(SOURCE_FILES
    smart_main.cpp
    ../test/method_factory.cpp
)

set(target_name ${PROJECT_NAME}_sample)
add_executable(${target_name} ${SOURCE_FILES})
target_link_libraries(${target_name} ${PROJECT_NAME} vioplugin)
