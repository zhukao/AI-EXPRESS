cmake_minimum_required(VERSION 2.8)
# 项目名称
project(stage2)

set(BASE_LIBRARIES xstream-framework hobotlog jsoncpp pthread)

set(SOURCE_FILES method/b_box_filter.cc method_factory.cc)

add_executable(stage2_sync_main sync_main.cc ${SOURCE_FILES})
target_link_libraries(stage2_sync_main ${BASE_LIBRARIES})

