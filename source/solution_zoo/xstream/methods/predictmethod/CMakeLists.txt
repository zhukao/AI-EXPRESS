cmake_minimum_required(VERSION 2.8)
project(PredictMethod)

include_directories(
        include
)

link_directories(${CMAKE_SOURCE_DIR}/lib/)

set(SOURCE_FILES
        src/PredictMethod.cc
        src/PredictConst.cc
        src/Predictors/DetectPredictor.cc
        src/Predictors/ClassifyPredictor.cc
        src/Predictors/Predictors.cc
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

set(DYNAMIC_DEPS_LIBS bpu_predict cnn_intf hbrt_bernoulli_aarch64 cam vio)
target_link_libraries(${PROJECT_NAME} ${DYNAMIC_DEPS_LIBS})

if(NOT ${PARENT_BUILD})
  set(OUTPUT_ROOT ${CMAKE_SOURCE_DIR}/output/)
endif()

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)

#add_subdirectory(test)

install(FILES ${PROJECT_SOURCE_DIR}/include/PredictMethod/PredictMethod.h
        DESTINATION ${MY_OUTPUT_ROOT}/include)

#install(DIRECTORY ${PROJECT_SOURCE_DIR}/configs
#        DESTINATION ${MY_OUTPUT_ROOT}/)

install(TARGETS ${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/lib)
