cmake_minimum_required(VERSION 3.15)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
)

if (${X86_ARCH})
  link_directories(
    ${PROJECT_SOURCE_DIR}/third_party/hobotlog/lib
  )
else ()
  link_directories(
    ../../../../../deps/hobotlog/lib
  )
endif()

# all source files of sample code
set(SOURCE_FILES
  sample_plugin.cpp
)

## base deps
if (PLATFORM_WIN)
  set(BASE_LIBRARIES hobotlog.lib)
else ()
  set(BASE_DEPS -ldl -lm -lpthread -lrt)
  set(BASE_LIBRARIES ${BASE_LIBRARIES} libhobotlog.a)
  set(BASE_LIBRARIES ${BASE_LIBRARIES} ${BASE_DEPS})
endif ()

# hobot_util.cmake已经设置了在build/bin中生成可执行文件，默认应该在sample目录里生成
add_executable(sample_plugin ${SOURCE_FILES})
target_link_libraries(sample_plugin
  PUBLIC ${PROJECT_NAME} ${BASE_LIBRARIES}
)
