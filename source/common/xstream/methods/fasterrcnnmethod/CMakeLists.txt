cmake_minimum_required(VERSION 2.8)
project(FasterRCNNMethod)

include_directories(
        include
        include/FasterRCNNMethod
)

link_directories(${CMAKE_SOURCE_DIR}/lib/)

set(SOURCE_FILES
        src/faster_rcnn.cpp
        src/faster_rcnn_imp.cpp
        src/yuv_utils.cc
        src/dump.cpp
        )

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

set(DYNAMIC_DEPS_LIBS bpu_predict cnn_intf hbrt_bernoulli_aarch64 cam vio)
target_link_libraries(${PROJECT_NAME} ${DYNAMIC_DEPS_LIBS})

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)

install(FILES ${PROJECT_SOURCE_DIR}/include/FasterRCNNMethod/FasterRCNNMethod.h
        DESTINATION ${MY_OUTPUT_ROOT}/include/FasterRCNNMethod)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/configs
        DESTINATION ${MY_OUTPUT_ROOT}/)

install(TARGETS ${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/lib)

add_subdirectory(example)
add_subdirectory(test)
