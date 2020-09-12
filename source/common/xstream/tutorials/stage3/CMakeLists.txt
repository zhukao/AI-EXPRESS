cmake_minimum_required(VERSION 2.8)
# 项目名称
project(stage3)

set(BASE_LIBRARIES xstream-framework hobotlog jsoncpp pthread)

set(SOURCE_FILES
  method/passthrough_method.cc
  method/thread_safety_method.cc
  orderdata.cc
  method_factory.cc)

add_executable(stage3_thread_safety_reorder_main
  thread_safety_reorder_main.cc
  ${SOURCE_FILES})
target_link_libraries(stage3_thread_safety_reorder_main ${BASE_LIBRARIES})

