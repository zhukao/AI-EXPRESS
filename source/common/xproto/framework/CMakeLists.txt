cmake_minimum_required(VERSION 3.15)

# project name
project(xproto)

option(X86_ARCH "x86 architecture" ON)

# add header files path
include_directories(
  ${PROJECT_SOURCE_DIR}/include
)

# x86 architecture
if (${X86_ARCH})
  message("tool chain:${CMAKE_CXX_COMPILER}")
  include_directories(
    ${PROJECT_SOURCE_DIR}/third_party/hobotlog/include
  )
# arm architecture
else ()
  set(CMAKE_CXX_COMPILER /opt/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++)
  message("tool chain:${CMAKE_CXX_COMPILER}")
  include_directories(
    ../../../../deps/hobotlog/include
  )
endif()

# header files
set(PROJECT_INC
  "include/xproto/manager/msg_manager.h"
  "include/xproto/message/pluginflow/flowmsg.h"
  "include/xproto/message/pluginflow/msg_registry.h"
  "include/xproto/plugin/xplugin.h"
  "include/xproto/plugin/xpluginasync.h"
  "include/xproto/threads/threadpool.h"
)

# source files
set(PROJECT_SRC
  "src/message/pluginflow/msg_registry.cpp"
  "src/plugin/xplugin.cpp"
  "src/plugin/xpluginasync.cpp"
  "src/threads/threadpool.cpp"
  "src/utils/gtest_helper.cpp"
  "src/utils/time_helper.cpp"
)

set(SOURCE_FILES
  ${PROJECT_SRC}
  ${PROJECT_INC}
)

set(BUILD_SHARED_LIBS true)
add_library(${PROJECT_NAME} ${SOURCE_FILES})

add_subdirectory(test)
add_subdirectory(sample)

# make install
set(OUTPUT_ROOT ${PROJECT_SOURCE_DIR}/output)
set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/xproto)
install(DIRECTORY
        ${CMAKE_SOURCE_DIR}/build/lib/
        DESTINATION ${MY_OUTPUT_ROOT}/lib/)
install(DIRECTORY
        ${PROJECT_SOURCE_DIR}/include/xproto/
        DESTINATION ${MY_OUTPUT_ROOT}/include/xproto)
