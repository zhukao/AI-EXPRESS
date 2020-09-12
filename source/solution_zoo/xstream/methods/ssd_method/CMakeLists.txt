cmake_minimum_required(VERSION 2.8)

project(ssd_method)

include_directories(include)
file(GLOB SOURCE_FILES src/ssd_method.cc 
src/ssd_predictor.cc
src/ssd_post_process_module.cc
src/ssd_result_post_process.cc
src/exptable.cc
src/Utils.cc)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/ssd_method/ssd_method.h
        DESTINATION ${MY_OUTPUT_ROOT}/include/)
install(TARGETS ${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/lib)

add_subdirectory(test)