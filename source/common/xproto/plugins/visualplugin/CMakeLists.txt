cmake_minimum_required(VERSION 2.8)

# 项目名称
project(visualplugin)

file(GLOB PROJECT_INC
    "include/visualplugin/*.h"
    "src/*.hh"
    "src/*.h"
)
file(GLOB PROJECT_SRC "src/*.cpp")

set(SOURCE_FILES ${PROJECT_SRC} ${PROJECT_INC})

include_directories(
    include
    src
)

if ((${PLATFORM_X3}) AND (${X3_MEDIA_CODEC}))
set(BASE_LIBRARIES opencv_world pthread  jsoncpp protobuf
    xproto_msgtype vioplugin smartplugin xstream-media_codec)
else ()
set(BASE_LIBRARIES opencv_world pthread  jsoncpp protobuf
    xproto_msgtype vioplugin smartplugin)
endif()
set(LIVE_LIB BasicUsageEnvironment groupsock liveMedia UsageEnvironment turbojpeg)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${BASE_LIBRARIES} ${LIVE_LIB})

#add_subdirectory(test)

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${MY_OUTPUT_ROOT}/include/${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} DESTINATION ${MY_OUTPUT_ROOT}/lib)
# install configs, some scripts will depends this
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/configs/
    DESTINATION ${MY_OUTPUT_ROOT}/config/)

