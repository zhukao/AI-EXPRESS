include_directories(
        ${PROJECT_SOURCE_DIR}/include
)

set(SOURCE_FILES
        src/example.c
)

add_executable(xstream_image_tools_example ${SOURCE_FILES})

target_link_libraries(xstream_image_tools_example ${PROJECT_NAME} libturbojpeg.a libyuv.a ${OpenCVLib} ${OpenCVDeps} pthread dl stdc++)
