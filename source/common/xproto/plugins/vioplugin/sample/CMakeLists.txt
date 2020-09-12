cmake_minimum_required(VERSION 2.8)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

set(SOURCE_FILES sample.cpp)

set(BASE_LIBRARIES vision_type_util vio cam xproto hobotlog
    protobuf jsoncpp dl m pthread rt)

# 添加依赖
set(target_name ${PROJECT_NAME}_sample)
add_executable(${target_name} ${SOURCE_FILES})
target_link_libraries(${target_name} ${PROJECT_NAME})
