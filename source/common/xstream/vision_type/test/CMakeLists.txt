include_directories(
        ${PROJECT_SOURCE_DIR}/include
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -fPIC")

file(GLOB_RECURSE TEST_SOURCE_SRC "src/*.cpp" "src/*.cc" "src/*.c" "*.cpp")

add_executable(gtest_vision_type ${TEST_SOURCE_SRC})

add_executable(c_example c_example.c)

set(OpenCVLib opencv_world)
# set(OpenCVDeps libjasper libjpeg-turbo libpng libprotobuf libtiff libwebp quirc tegra_hal  zlib)

target_link_libraries(gtest_vision_type vision_type_util ${OpenCVLib} ${OpenCVDeps} gtest pthread hobotsdk hobotthread dl)
target_link_libraries(c_example vision_type_util ${OpenCVLib} ${OpenCVDeps} gtest pthread hobotsdk hobotthread dl)
