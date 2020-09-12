file(GLOB TEST_SOURCE_SRC "src/*.cpp")
set(XSTREAM_LB -Wl,--whole-archive xstream-framework -Wl,--no-whole-archive)

add_executable(${PROJECT_NAME}_test ${TEST_SOURCE_SRC})

target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} ${XSTREAM_LB}
    hobotlog jsoncpp pthread dl rt gtest)
