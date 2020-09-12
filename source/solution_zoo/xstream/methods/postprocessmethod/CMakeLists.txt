cmake_minimum_required(VERSION 2.8)
project(PostProcessMethod)

include_directories(
        include
)

link_directories(${CMAKE_SOURCE_DIR}/lib/)

set(SOURCE_FILES
        src/PostProcessor/DetectConst.cc
        src/PostProcessor/DetectPostProcessor.cc
        src/PostProcessor/PostProcessor.cc
        src/PostProcessConst.cc
        src/PostProcessMethod.cc
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

set(DYNAMIC_DEPS_LIBS bpu_predict cnn_intf hbrt_bernoulli_aarch64 cam vio)
target_link_libraries(${PROJECT_NAME} ${DYNAMIC_DEPS_LIBS})

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)

install(FILES ${PROJECT_SOURCE_DIR}/include/PostProcessMethod/PostProcessMethod.h
        DESTINATION ${MY_OUTPUT_ROOT}/include/PostProcessMethod)

#install(DIRECTORY ${PROJECT_SOURCE_DIR}/configs
#        DESTINATION ${MY_OUTPUT_ROOT}/)

install(TARGETS ${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/lib)
