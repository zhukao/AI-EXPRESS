cmake_minimum_required(VERSION 2.8)

# 项目名称
project(media_codec_test)
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)
set(SOURCE_FILES
        gtest_main.cc
        media_codec_test.cpp
        )

set(BASE_LIBRARIES pthread dl rt)
set(X3_HAPI_LIB isp iar isp_algo tinyalsa multimedia z avformat hbmedia)
set(DepsLib xstream-media_codec hobotlog gtest)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
    ${DepsLib}
    ${X3_HAPI_LIB}
    ${BASE_LIBRARIES})
