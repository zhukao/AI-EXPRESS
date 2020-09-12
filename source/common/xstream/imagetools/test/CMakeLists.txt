include_directories(
        ${PROJECT_SOURCE_DIR}/include
)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.cc)

add_executable(gtest_imagetools ${SOURCE_FILES})
target_link_libraries(gtest_imagetools ${PROJECT_NAME} ${OpenCVLib} ${OpenCVDeps} hobotlog pthread gtest)

