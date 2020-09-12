cmake_minimum_required(VERSION 2.8)

# 项目名称
project(face_body_multisource)

# 头文件路径信息
file(GLOB PROJECT_INC
    "include/*.h"
    "include/*.hpp"
    "include/smartplugin/*.h"
    "include/smartplugin/utils/*.h"
    #"include/horizon/vision/*.h"
    "include/smartplugin/message/*.h"
)

# 源文件路径信息
file(GLOB PROJECT_SRC
    "src/solution_main.cpp"
    "src/smartplugin/*.cpp"
    "src/smartplugin/message/*.cc"
    "src/smartplugin/xstream/*.cpp"
)

set(SOURCE_FILES
    ${PROJECT_SRC}
    ${PROJECT_INC}
)

include_directories(
    include
    include/smartplugin
    include/smartplugin/message
    smartplugin
)

set(REMOVE_INC_DIR
    ${CMAKE_SOURCE_DIR}/xsdk/common/xproto/plugins/smartplugin/include
    ${CMAKE_SOURCE_DIR}/source/common/xproto/plugins/smartplugin/include
)

# 删除原来的smartplugin路径
get_property(current_include_dirs DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
list(REMOVE_ITEM current_include_dirs ${REMOVE_INC_DIR})
set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES ${current_include_dirs})

#add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
add_executable(${PROJECT_NAME} ${SOURCE_FILES} src/main.cpp)

set(Method_LIBS
    FasterRCNNMethod
    CNNMethod
    MOTMethod
    MergeMethod
    SnapShotMethod
    GradingMethod
)

set(BASE_LIBRARIES hobotlog xstream-framework vision_type_util
    jsoncpp protobuf vioplugin xproto_msgtype xproto)

set(OpenCVLib opencv_world)
# set(OPERATION_LIBRARIES ipc_tracking xstream-imagetools )
target_link_libraries(${PROJECT_NAME} ${BASE_LIBRARIES} ${OPERATION_LIBRARIES}
    ${Method_LIBS} ${OpenCVLib} pthread bpu_predict)

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)

add_subdirectory(test)

install(DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/config/
        DESTINATION ${MY_OUTPUT_ROOT}/configs/)
install(TARGETS ${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/)

