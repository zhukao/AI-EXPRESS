cmake_minimum_required(VERSION 2.8)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
)


# test case
file(GLOB TESTCASE_SRC 
    test_*.cpp
)

set(SOURCE_FILES
    ${PROJECT_SRC}
    gtest_main.cc
    ${TESTCASE_SRC}
)

# 添加依赖
set(BASE_LIBRARIES ${BASE_LIBRARIES} libgtest.a libhobotlog.a)

add_executable(${PROJECT_NAME}_test ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}_test ${BASE_LIBRARIES} ${Method_LIBS} ${OpenCVLib} pthread bpu_predict)

install(TARGETS ${PROJECT_NAME}_test
        DESTINATION ${MY_OUTPUT_ROOT}/)

