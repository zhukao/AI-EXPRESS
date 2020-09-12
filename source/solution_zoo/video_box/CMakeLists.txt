cmake_minimum_required(VERSION 2.8)
project(video_box)

# 头文件路径信息
file(GLOB PROJECT_INC
    "include/*.h"
)

# 源文件路径信息
file(GLOB PROJECT_SRC
    "src/*.cpp"
    "src/mediapipemanager/*.cpp"
    "src/rtspclient/*.cpp"
    "src/rtspplugin/*.cpp"
    "src/smartplugin_box/*.cpp"
)

set(SOURCE_FILES
    ${PROJECT_SRC}
    ${PROJECT_INC}
)

include_directories(
    include
)

link_directories(${CMAKE_SOURCE_DIR}/x3_prebuilt/lib
                     ${CMAKE_SOURCE_DIR}/x3_prebuilt/lib/hbmedia)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

if (${PLATFORM_X3})
set(BASE_LIBRARIES FasterRCNNMethod CNNMethod
xstream-framework xproto xproto_msgtype MOTMethod FilterMethod MergeMethod vote_method BehaviorMethod
        FilterSkipFrameMethod
        plate_vote_method
        vote_method
        VehiclePlateMatchMethod
        bpu_predict ipc_tracking xstream-imagetools hobotlog vision_type_util jsoncpp opencv_world vio cam pthread)

set(LIVE_LIB BasicUsageEnvironment groupsock liveMedia UsageEnvironment turbojpeg)

set(X3_HAPI_LIB isp iar isp_algo tinyalsa multimedia hbmedia z avformat )
endif()

target_link_libraries(${PROJECT_NAME} ${BASE_LIBRARIES} ${LIVE_LIB} ${X3_HAPI_LIB} )

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)
install(DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/configs/
        DESTINATION ${MY_OUTPUT_ROOT}/configs/)
install(TARGETS ${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/)
