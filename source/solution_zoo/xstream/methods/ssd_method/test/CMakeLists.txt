file(GLOB TEST_SOURCE_SRC "src/*.cpp")
set(XSTREAM_LB -Wl,--whole-archive xstream-framework bpu_predict -Wl,--no-whole-archive)
set(OpenCVLib viowrapper opencv_world libcam.so libvio.so)

add_executable(${PROJECT_NAME}_test ${TEST_SOURCE_SRC})

target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} ${XSTREAM_LB} ${OpenCVLib}
    hobotlog jsoncpp pthread dl rt gtest hbrt_bernoulli_aarch64)