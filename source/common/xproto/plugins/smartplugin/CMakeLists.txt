cmake_minimum_required(VERSION 2.8)

# 项目名称
project(smartplugin)

# 添加头文件路径
include_directories(
    include
)

file(GLOB PROJECT_SRC
    "src/smartplugin/*.cpp"
    "src/utils/*.cpp"
)
add_library(${PROJECT_NAME} SHARED ${PROJECT_SRC})

if (${PLATFORM_X3})
  set(X3_HAPI_LIB tinyalsa)
endif()

set(BASE_LIBRARIES xstream-framework xproto xproto_msgtype
    vision_type_util hobotlog jsoncpp protobuf opencv_world
    CNNMethod FasterRCNNMethod MOTMethod yuv turbojpeg
)
target_link_libraries(${PROJECT_NAME} ${BASE_LIBRARIES} ${X3_HAPI_LIB})

add_subdirectory(test)
add_subdirectory(sample)

# make install才会安装
set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/
    DESTINATION ${MY_OUTPUT_ROOT}/include/${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} DESTINATION ${MY_OUTPUT_ROOT}/lib)
