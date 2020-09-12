cmake_minimum_required(VERSION 2.8)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

set(SOURCE_FILES
    gtest_main.cpp
    gtest_msg.cpp
)

# 添加依赖
set(BASE_LIBRARIES ${PROJECT_NAME}  gtest dl m pthread rt)

set(target_name ${PROJECT_NAME}_test)
add_executable(${target_name} ${SOURCE_FILES})
target_link_libraries(${target_name} ${BASE_LIBRARIES})
