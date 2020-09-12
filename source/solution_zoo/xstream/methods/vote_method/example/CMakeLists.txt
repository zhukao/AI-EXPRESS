cmake_minimum_required(VERSION 2.8)

include_directories(
    include
)

set(XSTREAM_LB -Wl,--whole-archive xstream-framework -Wl,--no-whole-archive)

add_executable(${target_name}_example
    main.cpp
    ../test/src/method_factory.cpp
    test_anti_spf_calculate.cpp)

target_link_libraries(${target_name}_example
    ${target_name} hobotlog jsoncpp pthread dl rt
)
