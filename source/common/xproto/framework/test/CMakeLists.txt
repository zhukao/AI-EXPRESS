cmake_minimum_required(VERSION 3.15)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
)

if (${X86_ARCH})
  link_directories(
    ${PROJECT_SOURCE_DIR}/third_party/hobotlog/lib
  )
else ()
  include_directories(
    ../../../../../deps/gtest/include
  )

  link_directories(
    ../../../../../deps/gtest/lib
    ../../../../../deps/hobotlog/lib
  )
endif()

# test case的所有cpp文件
set(SOURCE_FILES
  gtest_main.cc
  test_api.cpp
  test_xplugin.cpp
)
# 添加依赖
## base deps
if (PLATFORM_WIN)
  set(BASE_LIBRARIES gtest.lib hobotlog.lib)
else ()
  set(BASE_DEPS -ldl -lm -lpthread -lrt)
  set(BASE_LIBRARIES ${BASE_LIBRARIES} libgtest.a libhobotlog.a)
  set(BASE_LIBRARIES ${BASE_LIBRARIES} ${BASE_DEPS})
endif ()

# hobot_util.cmake已经设置了在build/bin中生成可执行文件，默认应该在test目录里生成
add_executable(unit_test ${SOURCE_FILES})
target_link_libraries(unit_test
  PUBLIC ${PROJECT_NAME} ${BASE_LIBRARIES}
)
