include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

file(GLOB SOURCE_FILES "*.cc" "*.cpp" "*.hpp")

set(OpenCVLib opencv_world protobuf dl m pthread rt)
set(BASE_LIBRARIES ${OpenCVLib} ${PROJECT_NAME}
    xstream-framework gtest jsoncpp hobotlog)

add_executable(vehicle_snap_method_test ${SOURCE_FILES})
target_link_libraries(vehicle_snap_method_test ${BASE_LIBRARIES})

