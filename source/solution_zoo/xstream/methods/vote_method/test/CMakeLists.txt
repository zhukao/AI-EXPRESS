set(TEST_SOURCE_SRC
    src/gtest_main.cpp
    src/method_factory.cpp
    src/vote_method_test.cpp
)

set(XSTREAM_LB -Wl,--whole-archive xstream-framework -Wl,--no-whole-archive)

add_executable(${target_name}_test ${TEST_SOURCE_SRC})

target_link_libraries(${target_name}_test ${target_name} ${XSTREAM_LB}
    hobotlog jsoncpp pthread dl rt gtest)
