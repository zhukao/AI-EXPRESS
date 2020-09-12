cmake_minimum_required(VERSION 2.8)

include_directories(
        include
)

set(SOURCE_FILES
        ${CMAKE_CURRENT_LIST_DIR}/gtest_basic.cpp
        ${CMAKE_CURRENT_LIST_DIR}/gtest_main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/method_factory.cpp
        )

add_executable(gtest_behavior ${SOURCE_FILES})
target_link_libraries(gtest_behavior BehaviorMethod ${BASE_LIBRARIES} gtest pthread dl rt)

