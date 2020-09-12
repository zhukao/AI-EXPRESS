cmake_minimum_required(VERSION 2.8)

# 项目名称
project(websocketplugin)
set(CMAKE_CXX_STANDARD 11)
option(RELEASE_LIB "build version of release" ON)

file(GLOB PROJECT_INC
    "include/websocketplugin/*.h"
    "include/websocketplugin/message/*.h"
    "include/websocketplugin/server/*.h"
    "include/websocketplugin/utils/*.h"
)

file(GLOB PROJECT_SRC
    "src/*.cpp"
    "src/server/*.cpp"
    "src/utils/*.cpp"
    "src/websocketplugin/*.cpp"
)

set(SOURCE_FILES ${PROJECT_SRC} ${PROJECT_INC})

include_directories(
    include
    src
)

# 添加头文件路径
include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/third_party/include
)
link_directories(
        ${PROJECT_SOURCE_DIR}/third_party/lib
)
# 头文件路径信息

if ((${PLATFORM_X3}) AND (${X3_MEDIA_CODEC}))
set(BASE_LIBRARIES opencv_world pthread jsoncpp protobuf
    xproto_msgtype vioplugin smartplugin xstream-media_codec uWS ssl crypto z)
else ()
set(BASE_LIBRARIES opencv_world pthread jsoncpp protobuf
    xproto_msgtype vioplugin smartplugin uWS ssl crypto z)
endif()
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${BASE_LIBRARIES})

#add_subdirectory(test)
#add_subdirectory(sample)

set(OUTPUT_ROOT ${PROJECT_SOURCE_DIR}/output)
set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${MY_OUTPUT_ROOT}/include/${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} DESTINATION ${MY_OUTPUT_ROOT}/lib)
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/configs/
    DESTINATION ${MY_OUTPUT_ROOT}/config/)
