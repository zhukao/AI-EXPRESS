cmake_minimum_required(VERSION 2.8)
project(xwarehouse_sample)

set(CPP_FILES main.cpp)
add_executable(${PROJECT_NAME} ${CPP_FILES})

target_link_libraries( ${PROJECT_NAME} xwarehouse )

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)
install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${MY_OUTPUT_ROOT}/)
