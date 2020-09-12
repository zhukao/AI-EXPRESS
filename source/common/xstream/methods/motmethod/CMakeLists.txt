cmake_minimum_required(VERSION 2.8)
project(MOTMethod)

option(ENABLE_IOU2 "build iou 2.0" ON)
if(${ENABLE_IOU2})
    add_definitions(-DENABLE_IOU2)
endif()

include_directories(
        include
)

set(BASE_LIBRARIES hobotlog rt dl pthread)

if (${ENABLE_IOU2})
    set(IOU2_SRC_FILE "src/method/MOT/IOU2.cpp")
    set(IOU2_DEP_LIB "iou_based_mot")
endif()

set(SOURCE_FILES
        src/method/MOTMethod.cpp
        src/method/MOT/IOU.cpp
        ${IOU2_SRC_FILE}
#        src/method/MOT/ReID.cpp
)

set(MOT_LIBRARIES
#        multi_obj_tracking_lite
         ipc_tracking
         ${IOU2_DEP_LIB}
#        mxnet_predictor
#        haes.a
#        hcrypto.a
         hobotsdk hobotthread
#        mxnet_shared
#        xstream_basedata_helper
#        cudnn7
        )
add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
                      ${MOT_LIBRARIES}
#                      ${OpenCV_LIBS}
                      ${BASE_LIBRARIES} )

if(NOT ${PARENT_BUILD})
set(OUTPUT_ROOT ${CMAKE_SOURCE_DIR}/output/)
endif()

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)

install(FILES include/${PROJECT_NAME}/MOTMethod.h
        DESTINATION ${MY_OUTPUT_ROOT}/include/${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/lib)

add_subdirectory(test)
add_subdirectory(example)
