cmake_minimum_required(VERSION 3.15)
project(xstream-framework
        LANGUAGES CXX
)

set(CMAKE_C_COMPILER /opt/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /opt/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++)

option(XSTREAM_HOBOTSDK_ENGINE "choose hobotsdk engine" OFF)

option(C_INTERFACE "add c interface" OFF)
if(C_INTERFACE)
    add_definitions(-DC_INTERFACE)
endif()

include_directories(
        include
        include/common
        include/log
        include/timer
)
include_directories(
        ../../../../deps/jsoncpp/include/
        ../../../../deps/hobotlog/include/
        ../vision_type/include/
)

include_directories(SYSTEM /usr/include/python2.7)

set(SOURCE_FILES
        src/common/com_func.cpp
        src/profiler.cpp
        src/xstream.cpp
        src/xstream_data.cpp
        src/xstream_config.cpp
        src/method.cpp
        )
if(C_INTERFACE)
    set(SOURCE_FILES ${SOURCE_FILES} src/xstream_capi.cpp src/xstream_capi_type.cpp src/xstream_c_api.cpp)
endif()

if(XSTREAM_HOBOTSDK_ENGINE)
    add_definitions(-DHOBOTSDK_ENGINE)
    include_directories(include/hobotxstream/hobotsdk)
    set(HOBOTSDK_ENGINE_SOURCES
        include/hobotxstream/hobotsdk/BaseMessage.h
        include/hobotxstream/hobotsdk/NodeV2.h
        include/hobotxstream/hobotsdk/schedulerV2.h
        include/hobotxstream/hobotsdk/ThreadSafeMethod.h
        include/hobotxstream/hobotsdk/base_modules/CollectModule.h
        include/hobotxstream/hobotsdk/base_modules/DefaultDispatchModule.h
        include/hobotxstream/hobotsdk/base_modules/DispatchModule.h
        include/hobotxstream/hobotsdk/base_modules/MethodModule.h
        include/hobotxstream/hobotsdk/base_modules/ReorderDispatchModule.h
        src/hobotsdk/NodeV2.cpp
        src/hobotsdk/schedulerV2.cpp
        src/hobotsdk/base_modules/CollectModule.cpp
        src/hobotsdk/base_modules/DefaultDispatchModule.cpp
        src/hobotsdk/base_modules/DispatchModule.cpp
        src/hobotsdk/base_modules/MethodModule.cpp
        src/hobotsdk/base_modules/ReorderDispatchModule.cpp
    )
    set(SOURCE_FILES ${SOURCE_FILES} ${HOBOTSDK_ENGINE_SOURCES})
else()
    set(NATIVE_ENGINE_SOURCES
    src/method_manager.cpp
    src/timer/timer.cpp
    src/node.cpp
    src/scheduler.cpp
    src/thread_pool.cpp
    )
    set(SOURCE_FILES ${SOURCE_FILES} ${NATIVE_ENGINE_SOURCES})
endif()

set(BUILD_SHARED_LIBS false)
add_library(xstream-framework ${SOURCE_FILES})

set(BUILD_SHARED_LIBS true)
add_library(xstream ${SOURCE_FILES})

target_link_libraries(xstream-framework
                      PUBLIC rt)
target_link_libraries(xstream
                      PUBLIC rt)

if(XSTREAM_HOBOTSDK_ENGINE)
    target_link_libraries(xstream-framework
                          PUBLIC hobotsdk hobotthread jsoncpp hobotlog)
    target_link_libraries(xstream
                          PUBLIC hobotsdk hobotthread jsoncpp hobotlog)
endif()

add_subdirectory(example/bbox_filter)
add_subdirectory(test)

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/hobotxsdk
        DESTINATION ${MY_OUTPUT_ROOT}/include/
        FILES_MATCHING PATTERN "*.h")
install(FILES
        ${PROJECT_SOURCE_DIR}/include/hobotxstream/method.h
        ${PROJECT_SOURCE_DIR}/include/hobotxstream/method_factory.h
        ${PROJECT_SOURCE_DIR}/include/hobotxstream/profiler.h
        DESTINATION ${MY_OUTPUT_ROOT}/include/hobotxstream)

install(TARGETS     ${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/lib)

