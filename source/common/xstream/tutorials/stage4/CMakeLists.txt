cmake_minimum_required(VERSION 2.8)
# 项目名称
project(stage4)

# 设置头文件搜索路径
include_directories(./)

set(BASE_LIBRARIES xstream-framework hobotlog jsoncpp pthread)

set(SOURCE_FILES
  method/bbox.h
  method/bbox_filter.h
  method/bbox_filter.cc
  method/filter_param.h
  method/postbox_filter.h
  method/postbox_filter.cc)

add_executable(stage4_disable_method_example
  disable_method_main.cc
  method/method_factory.h
  method/method_factory.cc
  ${SOURCE_FILES})
target_link_libraries(stage4_disable_method_example ${BASE_LIBRARIES})

add_executable(stage4_pass_through_method_example
  pass_through_method_main.cc
  method/method_factory.h
  method/method_factory.cc
  ${SOURCE_FILES})
target_link_libraries(stage4_pass_through_method_example ${BASE_LIBRARIES})

add_executable(stage4_use_predefined_method_example
  use_predefined_method_main.cc
  method/method_factory.h
  method/method_factory.cc
  ${SOURCE_FILES})
target_link_libraries(stage4_use_predefined_method_example ${BASE_LIBRARIES})


add_executable(stage4_best_effort_pass_through_method_example
  best_effort_pass_through_method_main.cc
  method/method_factory.h
  method/method_factory.cc
  ${SOURCE_FILES})
target_link_libraries(stage4_best_effort_pass_through_method_example ${BASE_LIBRARIES})


add_executable(stage4_best_effort_pass_through_method2_example
  best_effort_pass_through_method_main_2.cc
  method/method_factory.h
  method/method_factory.cc
  ${SOURCE_FILES})
target_link_libraries(stage4_best_effort_pass_through_method2_example ${BASE_LIBRARIES})

