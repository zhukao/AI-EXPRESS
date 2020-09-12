cmake_minimum_required(VERSION 2.8)
# 项目名称
project(stage1)

set(BASE_LIBRARIES xstream-framework hobotlog jsoncpp pthread)

set(SOURCE_FILES method/b_box_filter.cc method_factory.cc)

add_executable(stage1_sync_main sync_main.cc ${SOURCE_FILES})
target_link_libraries(stage1_sync_main ${BASE_LIBRARIES})

add_executable(stage1_async_main async_main.cc ${SOURCE_FILES})
target_link_libraries(stage1_async_main ${BASE_LIBRARIES})

add_executable(stage1_update_param_main update_param_main.cc ${SOURCE_FILES})
target_link_libraries(stage1_update_param_main ${BASE_LIBRARIES})

