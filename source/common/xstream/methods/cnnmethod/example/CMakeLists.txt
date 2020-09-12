cmake_minimum_required(VERSION 2.8)

set(BASE_LIBRARIES jsoncpp pthread dl rt)
set(OpenCVLib opencv_world)
set(VioWrapperLib viowrapper)

set(XSTREAM_LB -Wl,--whole-archive xstream-framework -Wl,--no-whole-archive)
set(X2_PREBUILT_LIB bpu_predict libcam.so libvio.so libcnn_intf.so)

file(GLOB_RECURSE EXAMPLE_SOURCE_FILES *.cpp)

add_executable(CNNMethod_example
               ${EXAMPLE_SOURCE_FILES}
)
include_directories(include)

target_link_libraries(CNNMethod_example
  CNNMethod
  FasterRCNNMethod
  ${VioWrapperLib}
  ${XSTREAM_LB}
  ${X2_PREBUILT_LIB}
  ${BASE_LIBRARIES}
  ${OpenCVLib}
  hobotlog
  libturbojpeg.a
  libyuv.a
  hbrt_bernoulli_aarch64
)

#install(TARGETS example
#  DESTINATION ${MY_OUTPUT_ROOT}/bin)
