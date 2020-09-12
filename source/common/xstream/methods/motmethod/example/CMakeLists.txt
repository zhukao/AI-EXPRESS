cmake_minimum_required(VERSION 2.8)

option(SHARED_LIB "build shared lib" OFF)

set(SOURCE_FILES
  ${CMAKE_CURRENT_LIST_DIR}/method_factory.cpp
)

set(BASE_LIBRARIES jsoncpp xstream-framework hobotlog rt pthread dl)

set(MOT_LIBRARIES
        #        multi_obj_tracking_lite
        hobotsdk
        hobotthread
        #        mxnet_shared
        #        xstream_basedata_helper
        #        cudnn7
        )

#set(OpenCV_LIBS opencv_highgui opencv_imgcodecs opencv_calib3d opencv_imgproc opencv_video opencv_videoio
#        opencv_core libpng z libtiff libwebp libpng libjpeg libjasper IlmImf opencv_features2d opencv_flann
#        opencv_imgcodecs opencv_ml opencv_objdetect opencv_photo opencv_shape
#        opencv_stitching opencv_superres opencv_videostab)

if (${SHARED_LIB})
    add_library(mot_example_shared SHARED ${SOURCE_FILES})
    target_link_libraries(mot_example_shared ${CMAKE_PROJECT_NAME} -Wl,--whole-archive xstream-framework -Wl,--no-whole-archive
            ${MOT_LIBRARIES}
#            ${OpenCV_LIBS}
            ${BASE_LIBRARIES})

    add_executable(iou_mot_example iou_mot_example.cpp)
    target_link_libraries(iou_mot_example mot_example_shared)

    #add_executable(reid_mot_example reid_mot_example.cpp)
    #target_link_libraries(reid_mot_example mot_example_shared)
  if (${ENABLE_IOU2}) 
    add_executable(iou_mot_example2 iou_mot_example_2.cpp)
    target_link_libraries(iou_mot_example2 mot_example_shared)
  endif()
else()
    add_executable(iou_mot_example ${SOURCE_FILES} iou_mot_example.cpp)
    target_link_libraries(iou_mot_example
            MOTMethod
#            ${OpenCV_LIBS}
            ${BASE_LIBRARIES})

  if (${ENABLE_IOU2})
    add_executable(iou_mot_example2 ${SOURCE_FILES} iou_mot_example_2.cpp)
    target_link_libraries(iou_mot_example2
           MOTMethod
           ${BASE_LIBRARIES})
  endif()
endif()
