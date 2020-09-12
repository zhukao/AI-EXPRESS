cmake_minimum_required(VERSION 2.8.12)
project(xstream_python_api)

set(CMAKE_STRIP "/opt/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-strip")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g")
set(METHOD_WRAPPER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/method)
set(VISION_TYPE_WRAPPER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/vision_type)

# xstream python binding library
pybind11_add_module(xstream_internal
    ${CMAKE_CURRENT_SOURCE_DIR}/src/xstream.cpp
)

# method factory
add_library(method_factory SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/method_factory.cpp
    ${METHOD_WRAPPER_SRC}/bbox_filter/b_box_filter.cc
)

# method factory dependency
target_link_libraries(method_factory PRIVATE
    FasterRCNNMethod
    MOTMethod
    CNNMethod
    SnapShotMethod
    GradingMethod
    MergeMethod
    FilterSkipFrameMethod
    plate_vote_method
    vote_method
    VehiclePlateMatchMethod
    vehicle_snap_method
)

target_link_libraries(xstream_internal PRIVATE
    method_factory
    xstream-framework
    bpu_predict
    ipc_tracking
    hobotlog
    jsoncpp
    opencv_world
)

# version type
pybind11_add_module(vision_type
    ${VISION_TYPE_WRAPPER_SRC}/vision_type.cpp
    ${VISION_TYPE_WRAPPER_SRC}/bbox_type.cpp
    #src/vision_type/image_frame.cpp
)
target_link_libraries(vision_type PRIVATE
    xstream-framework
)
