cmake_minimum_required(VERSION 2.8)

# 项目名称
project(xstream-media_codec)

# 添加头文件路径
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

file(GLOB PROJECT_INC
    "include/media_codec/*.h"
    "include/utils/*.h"
)

file(GLOB PROJECT_SRC
    "src/media_codec/*.cpp"
    "src/utils/*.cpp"
)

set(SOURCE_FILES ${PROJECT_SRC} ${PROJECT_INC})

set(X3_HAPI_LIB isp iar isp_algo tinyalsa multimedia z avformat hbmedia)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${X3_HAPI_LIB})
add_subdirectory(test)

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${MY_OUTPUT_ROOT}/include/)
install(TARGETS ${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/lib)
