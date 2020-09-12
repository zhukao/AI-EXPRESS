cmake_minimum_required(VERSION 2.8)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)
set(SOURCE_FILES sample.cpp)

# 添加依赖
set(BASE_DEPS dl m pthread rt)
set(target_name ${PROJECT_NAME}_sample)
add_executable(${target_name} ${SOURCE_FILES})
target_link_libraries(${target_name} ${PROJECT_NAME} ${BASE_DEPS})

