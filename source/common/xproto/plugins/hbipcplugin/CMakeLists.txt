cmake_minimum_required(VERSION 2.8)

# 项目名称
project(hbipcplugin)

# 添加头文件路径
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# 头文件路径信息
file(GLOB PROJECT_INC
    "include/hbipcplugin/*.h"
    "include/utils/*.h"
)

# 源文件路径信息
file(GLOB PROJECT_SRC
    "src/hbipcplugin/*.cpp"
    "src/utils/*.cpp"
)

set(SOURCE_FILES ${PROJECT_SRC} ${PROJECT_INC})

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set(BASE_LIBRARIES lib_hbipc_cp.so xproto xproto_msgtype
    protobuf hobotlog vision_type_util jsoncpp)
target_link_libraries(${PROJECT_NAME} ${BASE_LIBRARIES})

add_subdirectory(sample)
add_subdirectory(test)

# make install才会安装
set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${MY_OUTPUT_ROOT}/include/${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} DESTINATION ${MY_OUTPUT_ROOT}/lib)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/config/
    DESTINATION ${MY_OUTPUT_ROOT}/config/)
