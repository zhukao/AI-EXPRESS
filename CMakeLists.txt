cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_COMPILER /opt/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /opt/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++)

option(COVERAGE_TEST "open coverage build flag" OFF)
option(RELEASE_LIB "build version of release" ON)
option(EEPROM_ENABLED "Enable EEPROM Read/Write" OFF)
option(SUPPORT_RGB_NIR_MERGE "support rgb nir merge" OFF)
option(SMART_SHARED_LIB "shared_lib" OFF)
option(XSTREAM_HOBOTSDK_ENGINE "choose hobotsdk engine" ON)
option(PLATFORM_X2 "x2 platform" ON)
option(PLATFORM_X3 "x3 platform" OFF)
option(IOT_VIO_API "iot vio api" ON)
option(X3_MEDIA_CODEC "x3 media codec" ON)

project(build_all)

message("config types: ${CMAKE_CONFIGURATION_TYPES}")

add_definitions(-DHR_POSIX)
add_definitions(-DHR_LINUX)

set (VioPlugin source/common/xproto/plugins/vioplugin)

if (${RELEASE_LIB})
    set(CMAKE_BUILD_TYPE Release)
    message(STATUS "BUILD TYPE:Release")
else ()
    set(CMAKE_BUILD_TYPE Debug)
    set(HOBOT_BUILD_DEBUG "TRUE")
    message(STATUS "BUILD TYPE:Debug")
endif ()

list(APPEND CMAKE_C_FLAGS "-Wall -Wno-unknown-pragmas -fPIC -fexceptions -Werror ")
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    list(APPEND CMAKE_C_FLAGS "-O3")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC ")
else ()
    list(APPEND CMAKE_C_FLAGS "-g -O0")
endif()

list(APPEND CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 ")
string(REGEX REPLACE ";" " " CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
string(REGEX REPLACE ";" " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/)
set(LIBRARY_OUTPUT_PATH  ${PROJECT_BINARY_DIR}/lib/)

if (${PLATFORM_X2})
    message("X2 is defined##############################################")
    add_definitions(-DX2)
    include_directories(
        deps/x2_prebuilt/include
    )
    link_directories(${CMAKE_SOURCE_DIR}/deps/x2_prebuilt/lib/)
    set (ARCHITECTURE x2)
elseif (${PLATFORM_X3})
    message("X3 is defined##############################################")
    add_definitions(-DX3)
    include_directories(
        deps/x3_prebuilt/include
        deps/x3_prebuilt/include/hbmedia
    )
    if (${IOT_VIO_API})
        add_definitions(-DX3_IOT_VIO)
    else ()
        add_definitions(-DX3_X2_VIO)
    endif ()
    add_definitions(-DJ3_MEDIA_LIB)
    if (${X3_MEDIA_CODEC})
    add_definitions(-DX3_MEDIA_CODEC)
    endif()
    set (VioPlugin source/common/xproto/plugins/iotvioplugin)
    set (IOT_VIO_SRC ${CMAKE_SOURCE_DIR}/source/common/xproto/plugins/iotvioplugin)
    message("x3 VioPlugin:${VioPlugin}#############")
    link_directories(${CMAKE_SOURCE_DIR}/deps/x3_prebuilt/lib/
                    ${CMAKE_SOURCE_DIR}/deps/x3_prebuilt/lib/hbmedia)
    set (ARCHITECTURE x3)
else ()
    message(FATAL_ERROR "must set platform to PLATFORM_X2 or PLATFORM_X3")
    return()
endif ()

include_directories(
        # solutions
        source/solution_zoo/face/include
        # frameworks
        source/common
        source/common/xstream/framework/include
        source/common/xproto/framework/include/
        source/common/xproto/msgtype/include/
        ${VioPlugin}/include/
        source/common/xproto/plugins/smartplugin/include/
        source/common/xproto/plugins/visualplugin/include/
        source/common/xproto/plugins/hbipcplugin/include/
        source/common/xproto/plugins/websocketplugin/include/
        source/common/xproto/plugins/uvcplugin/include/
        source/common/xstream/vision_type/include
        source/common/xstream/media_codec/include
        source/common/viowrapper/include
        # methods
        source/common/xstream/methods/cnnmethod/include
        source/common/xstream/methods/fasterrcnnmethod/include
        source/solution_zoo/xstream/methods/gradingmethod/include
        source/solution_zoo/xstream/methods/filtermethod/include
        source/solution_zoo/xstream/methods/snapshotmethod/include
        source/solution_zoo/xstream/methods/mergemethod/include
        source/solution_zoo/xstream/methods/behavior_method/include
        source/solution_zoo/xstream/methods/predictmethod/include
        source/solution_zoo/xstream/methods/postprocessmethod/include
        source/common/xstream/methods/motmethod/include
        source/solution_zoo/xstream/methods/vehicle_snap_method/include
        source/solution_zoo/xstream/methods/vehicle_plate_match_method/include
#        source/solution_zoo/xstream/methods/vehicle_road_relationship_method/include
        source/solution_zoo/xstream/methods/plate_vote_method/include
        source/solution_zoo/xstream/methods/vote_method/include
        source/solution_zoo/xstream/methods/FilterSkipFrameMethod/include
        source/solution_zoo/xstream/methods/ssd_method/include
        # other utilities
        source/common/xstream/imagetools/include
        deps/ipc_tracking/include
        deps/bpu_predict/${ARCHITECTURE}/include
        deps/jsoncpp/include
        deps/hobotlog/include
        deps/protobuf/include
        deps/gtest/include
        deps/opencv/include
        deps/libjpeg-turbo/include
        deps/libyuv/include
        deps/hobot/include
        deps/iou_based_mot/include
#        deps/road_model/include
#        deps/roadside_perception/include
#        deps/vehicle_behavior/include
#        deps/vehicle_road_relation/include
#        deps/param_calibration/include
        deps/zlib/include
        deps/live555/include
        deps/xwarehouse/include
        deps/uWS/include
        deps/openssl/include
)


link_directories(
        ${CMAKE_SOURCE_DIR}/deps/ipc_tracking/lib/
        ${CMAKE_SOURCE_DIR}/deps/bpu_predict/${ARCHITECTURE}/lib/
        ${CMAKE_SOURCE_DIR}/deps/jsoncpp/lib/
        ${CMAKE_SOURCE_DIR}/deps/hobotlog/lib/
        ${CMAKE_SOURCE_DIR}/deps/protobuf/lib/
        ${CMAKE_SOURCE_DIR}/deps/gtest/lib/
        ${CMAKE_SOURCE_DIR}/deps/opencv/lib/
        ${CMAKE_SOURCE_DIR}/deps/libjpeg-turbo/lib/
        ${CMAKE_SOURCE_DIR}/deps/libyuv/lib/
        ${CMAKE_SOURCE_DIR}/deps/hobot/lib/
        ${CMAKE_SOURCE_DIR}/deps/iou_based_mot/lib/
#        ${CMAKE_SOURCE_DIR}/deps/road_model/lib/
#        ${CMAKE_SOURCE_DIR}/deps/roadside_perception/lib/
#        ${CMAKE_SOURCE_DIR}/deps/vehicle_behavior/lib/
#        ${CMAKE_SOURCE_DIR}/deps/vehicle_road_relation/lib/
#        ${CMAKE_SOURCE_DIR}/deps/param_calibration/lib/
        ${CMAKE_SOURCE_DIR}/deps/zlib/lib/
        ${CMAKE_SOURCE_DIR}/deps/live555/lib/
        ${CMAKE_SOURCE_DIR}/deps/xwarehouse/lib/
        ${CMAKE_SOURCE_DIR}/deps/uWS/lib64/
        ${CMAKE_SOURCE_DIR}/deps/openssl/lib/
)

set(OUTPUT_ROOT ${CMAKE_SOURCE_DIR}/output/)


add_subdirectory(source/common/xproto/framework)
add_subdirectory(source/common/xproto/msgtype)
add_subdirectory(${VioPlugin})
add_subdirectory(source/common/xproto/plugins/smartplugin)
add_subdirectory(source/common/xproto/plugins/visualplugin)
if (${PLATFORM_X2})
add_subdirectory(source/common/xproto/plugins/hbipcplugin)
endif()
add_subdirectory(source/common/xproto/plugins/websocketplugin)
if (${PLATFORM_X3})
add_subdirectory(source/common/xproto/plugins/uvcplugin)
if (${X3_MEDIA_CODEC})
add_subdirectory(source/common/xstream/media_codec)
endif()
endif()
add_subdirectory(source/common/xstream/framework)
add_subdirectory(source/common/xstream/vision_type)
add_subdirectory(source/common/xstream/tutorials/stage1)
add_subdirectory(source/common/xstream/tutorials/stage2)
add_subdirectory(source/common/xstream/tutorials/stage3)
add_subdirectory(source/common/xstream/tutorials/stage4)
add_subdirectory(source/common/xstream/methods/fasterrcnnmethod)
add_subdirectory(source/common/xstream/imagetools)
add_subdirectory(source/common/viowrapper)
add_subdirectory(source/common/xstream/methods/cnnmethod)
add_subdirectory(source/common/xstream/methods/motmethod)

add_subdirectory(source/solution_zoo/xstream/methods/gradingmethod)
add_subdirectory(source/solution_zoo/xstream/methods/filtermethod)
add_subdirectory(source/solution_zoo/xstream/methods/mergemethod)
add_subdirectory(source/solution_zoo/xstream/methods/behavior_method)
add_subdirectory(source/solution_zoo/xstream/methods/snapshotmethod)
add_subdirectory(source/solution_zoo/xstream/methods/vehicle_snap_method)
add_subdirectory(source/solution_zoo/xstream/methods/vehicle_plate_match_method)
#add_subdirectory(source/solution_zoo/xstream/methods/vehicle_road_relationship_method)
add_subdirectory(source/solution_zoo/xstream/methods/plate_vote_method)
add_subdirectory(source/solution_zoo/xstream/methods/FilterSkipFrameMethod)
add_subdirectory(source/solution_zoo/xstream/methods/vote_method)
add_subdirectory(source/solution_zoo/xstream/methods/ssd_method)

add_subdirectory(source/solution_zoo/face/)
add_subdirectory(source/solution_zoo/body/)
#add_subdirectory(source/solution_zoo/vehicle/)
add_subdirectory(source/solution_zoo/face_body_multisource/)
if (${PLATFORM_X3})
    add_subdirectory(source/solution_zoo/video_box/)
    add_subdirectory(source/solution_zoo/xstream/methods/predictmethod)
    add_subdirectory(source/solution_zoo/xstream/methods/postprocessmethod)
endif()
# sample
add_subdirectory(source/solution_zoo/tools/xwarehouse/)
