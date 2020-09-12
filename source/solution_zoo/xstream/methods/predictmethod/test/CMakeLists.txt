include_directories(
        include
)

set(SOURCE_FILES
        gtest_main.cc
        apa_test.cpp
)

set(COMMON_DEPS
   -lpthread -ldl
)

set(BASE_LIBRARIES jsoncpp pthread dl rt)
set(OpenCVLib opencv_world)
set(XSTREAM_LB -Wl,--whole-archive xstream-framework -Wl,--no-whole-archive)
set(X2_PREBUILT_LIB bpu_predict libcam.so libvio.so libcnn_intf.so)
set(VioWrapperLib viowrapper)
add_executable(apa_test ${SOURCE_FILES})

if (${PLATFORM_WIN})
     set(CMAKE_CXX_FLAGS_RELEASE "/MT")
     set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
     target_link_libraries(apa_test debug gtestd)
     target_link_libraries(apa_test optimized gtest)
else()
     target_link_libraries(apa_test
                           ${COMMON_DEPS}
                           gtest
                           PredictMethod
                           ${VioWrapperLib}
                           ${XSTREAM_LB}
                           ${X2_PREBUILT_LIB}
                           ${BASE_LIBRARIES}
                           ${OpenCVLib}
                           hobotlog
                           hbrt_bernoulli_aarch64)
endif()
