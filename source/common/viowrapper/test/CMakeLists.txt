
set(SOURCE_FILES
        gtest_main.cc
        vio_wrapper_test.cpp
        )

set(COMMON_DEPS
   -lpthread -ldl
    )

set(BASE_LIBRARIES pthread dl rt)
set(OpenCVLib opencv_world)
#set(XSTREAM_LB -Wl,--whole-archive xstream-framework -Wl,--no-whole-archive)
set(X2_PREBUILT_LIB libcam.so libvio.so)
set(VioWrapperLib viowrapper)
add_executable(VIOWrapper_unit_test ${SOURCE_FILES})

if (${PLATFORM_WIN})
     set(CMAKE_CXX_FLAGS_RELEASE "/MT")
     set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
     target_link_libraries(VIOWrapper_unit_test debug gtestd)
     target_link_libraries(VIOWrapper_unit_test optimized gtest)
else()
     target_link_libraries(VIOWrapper_unit_test
                           ${COMMON_DEPS}
                           gtest
                           ${VioWrapperLib}
                           ${XSTREAM_LB}
                           ${X2_PREBUILT_LIB}
                           ${BASE_LIBRARIES}
                           ${OpenCVLib})
endif()
