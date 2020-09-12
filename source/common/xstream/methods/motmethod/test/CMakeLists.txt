cmake_minimum_required(VERSION 2.8)

option(SHARED_LIB "build shared lib" OFF)

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/include
)

set(SOURCE_FILES
  ${CMAKE_CURRENT_LIST_DIR}/method_factory.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/callback.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/iou_test.cpp
)

set(BASE_LIBRARIES jsoncpp.a xstream-framework hobotlog rt pthread dl)

#set(OpenCV_LIBS opencv_highgui opencv_imgcodecs opencv_calib3d opencv_imgproc opencv_video opencv_videoio
#        opencv_core libpng z libtiff libwebp libpng libjpeg libjasper IlmImf opencv_features2d opencv_flann
#        opencv_imgcodecs opencv_ml opencv_objdetect opencv_photo opencv_shape
#        opencv_stitching opencv_superres opencv_videostab)

if (${SHARED_LIB})
    set(MOT_LIBRARIES
            #        multi_obj_tracking_lite
            hobotsdk
            hobotthread
            #        mxnet_shared
            #        xstream_basedata_helper
            #        cudnn7
            )
    add_library(mot_gtest_shared SHARED ${SOURCE_FILES})
    target_link_libraries(mot_gtest_shared ${PROJECT_NAME} -Wl,--whole-archive xstream-framework -Wl,--no-whole-archive
            ${MOT_LIBRARIES}
            #        ${OpenCV_LIBS}
            ${BASE_LIBRARIES})

    add_executable(gtest_mot gtest_MOT_Method.cpp)
    target_link_libraries(gtest_mot mot_gtest_shared pthread gtest)
else()
    add_executable(gtest_mot ${SOURCE_FILES} gtest_MOT_Method.cpp)
    target_link_libraries(gtest_mot
            MOTMethod
            #            ${OpenCV_LIBS}
            ${BASE_LIBRARIES}
            pthread
            gtest)
endif()
