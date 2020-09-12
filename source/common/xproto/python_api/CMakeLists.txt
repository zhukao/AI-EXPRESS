cmake_minimum_required(VERSION 2.8)

project(xproto_python_api)

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
get_filename_component(GRAND_DIR ${PARENT_DIR} DIRECTORY)
message(${PARENT_DIR})
message(${GRAND_DIR})

set(CMAKE_STRIP "/opt/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-strip")

#set(ARCH_LIBS bpu_predict cnn_intf vio cam fb)
set(ARCH_LIBS vio cam)

include_directories(
				include
				${PARENT_DIR}/framework/include
				${PARENT_DIR}/msgtype/include
				${PARENT_DIR}/msgtype/include/xproto_msgtype/protobuf
#				${PARENT_DIR}/plugins/hbipcplugin/include
				${PARENT_DIR}/plugins/smartplugin/include
				${PARENT_DIR}/plugins/vioplugin/include
				pybind11/include
        x2_prebuilt/include
        ${GRAND_DIR}/xstream/vision_type/include
        ${GRAND_DIR}/xstream/framework/include
)

file(GLOB PYMODULE_INC
				"../framework/include/xproto/manager/*.h"
				"../framework/include/xproto/message/pluginflow/*.h"
				"../framework/include/xproto/plugin/*.h"
				"../framework/include/xproto/threads/*.h"
				"../framework/include/xproto/utils/*.h"
				"../msgtype/include/xproto_msgtype/*.h"
				"../msgtype/include/xproto_msgtype/protobuf/*.h"
				"../plugins/vioplugin/include/vioplugin/*.h"
#				"../plugins/hbipcplugin/include/hbipcplugin/*.h"
				"../plugins/vioplugin/include/utils/executor.h"
				"../plugins/smartplugin/include/smartplugin/*.h"
				"../plugins/smartplugin/include/horizon/vision/*.h"
)

file(GLOB PYMODULE_SRC
				"../framework/src/plugin/*.cpp"
				"../framework/src/threads/*.cpp"
				"../framework/src/utils/*.cpp"
				"../msgtype/src/protobuf/*.cc"
				"../plugins/vioplugin/src/vioplugin/*.cpp"
				"../plugins/vioplugin/src/utils/executor.cpp"
#				"../plugins/hbipcplugin/src/hbipcplugin/*.cpp"
				"../plugins/smartplugin/src/smartplugin/*.cpp"
)

#link_directories(
#        ${CMAKE_CURRENT_SOURCE_DIR}/libs
#)

set(SOURCE_FILES
				${PYMODULE_INC}
				${PYMODULE_SRC}
)

set(PYBIND11_PYTHON_VERSION 3.6)
set(OpenCVLib opencv_world)
set(BASE_LIBRARIES
    xstream-framework hobotsdk hobotthread
    vision_type_util hobotlog protobuf jsoncpp ${OpenCVLib} ${ARCH_LIBS})

#add_subdirectory(test)

#add_subdirectory(pybind11)
pybind11_add_module(native_xproto
				${SOURCE_FILES}
				src/smart_helper.cpp
				src/wrapper.cpp
        src/method_factory.cpp
)

target_link_libraries(native_xproto PRIVATE ${BASE_LIBRARIES})
