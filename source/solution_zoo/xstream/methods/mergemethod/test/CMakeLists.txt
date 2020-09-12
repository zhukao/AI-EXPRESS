cmake_minimum_required(VERSION 2.8)

include_directories(
        include
)

set(SOURCE_FILES
        ${CMAKE_CURRENT_LIST_DIR}/method_factory.cpp
        ${CMAKE_CURRENT_LIST_DIR}/gtest_main.cpp
        )

add_executable(gtest_head_face ${SOURCE_FILES} gtest_head_face.cpp)
target_link_libraries(gtest_head_face MergeMethod ${BASE_LIBRARIES} gtest pthread dl rt)

