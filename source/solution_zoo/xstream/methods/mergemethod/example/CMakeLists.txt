cmake_minimum_required(VERSION 2.8)

include_directories(
        ${CMAKE_CURRENT_LIST_DIR}/include
)

set(SOURCE_FILES
        ${CMAKE_CURRENT_LIST_DIR}/method_factory.cpp
        )

add_executable(merge_unit_test1 ${SOURCE_FILES} test_main1.cpp)
target_link_libraries(merge_unit_test1 MergeMethod ${BASE_LIBRARIES} gtest pthread dl rt)
add_executable(merge_unit_test2 ${SOURCE_FILES} test_main2.cpp)
target_link_libraries(merge_unit_test2 MergeMethod ${BASE_LIBRARIES} gtest pthread dl rt)

