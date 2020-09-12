#!/bin/bash
# copy runtime from release environment
set -x
ALL_PROJECT_DIR=$PWD
RELEASE_DIR=${ALL_PROJECT_DIR}/deploy
rm ${RELEASE_DIR} -rf
mkdir -p ${RELEASE_DIR}
ARCHITECTURE=$(cat platform.tmp)
## libs
mkdir ${RELEASE_DIR}/lib/
cp ${ALL_PROJECT_DIR}/build/lib/libxproto*.so ${RELEASE_DIR}/lib/ -rf
cp ${ALL_PROJECT_DIR}/build/lib/libvioplugin*.so ${RELEASE_DIR}/lib/ -rf
cp ${ALL_PROJECT_DIR}/build/lib/libsmartplugin*.so ${RELEASE_DIR}/lib/ -rf
cp ${ALL_PROJECT_DIR}/build/lib/libvisualplugin*.so ${RELEASE_DIR}/lib/ -rf
cp ${ALL_PROJECT_DIR}/build/lib/libwebsocketplugin*.so ${RELEASE_DIR}/lib/ -rf
if [ ${ARCHITECTURE} == "x3" ]
then
  cp ${ALL_PROJECT_DIR}/build/lib/libuvcplugin*.so ${RELEASE_DIR}/lib/ -rf
  cp ${ALL_PROJECT_DIR}/deps/x3_prebuilt/lib/libguvc*.so ${RELEASE_DIR}/lib/ -rf
  cp ${ALL_PROJECT_DIR}/build/lib/libxstream-media_codec*.so ${RELEASE_DIR}/lib/ -rf
  cp ${ALL_PROJECT_DIR}/build/lib/libmulti*.so ${RELEASE_DIR}/lib/ -rf
  cp ${ALL_PROJECT_DIR}/build/lib/libgdcplugin*.so ${RELEASE_DIR}/lib/ -rf
	cp ${ALL_PROJECT_DIR}/source/solution_zoo/apa/gdcplugin/deps/*.so ${RELEASE_DIR}/lib/ -rf
  cp ${ALL_PROJECT_DIR}/build/lib/libdisplayplugin*.so ${RELEASE_DIR}/lib/ -rf
	cp ${ALL_PROJECT_DIR}/source/solution_zoo/apa/displayplugin/deps/lib/*.so ${RELEASE_DIR}/lib/ -rf
  cp ${ALL_PROJECT_DIR}/build/lib/libcanplugin.so ${RELEASE_DIR}/lib/ -rf
fi
cp ${ALL_PROJECT_DIR}/deps/bpu_predict/${ARCHITECTURE}/lib/* ${RELEASE_DIR}/lib/ -rf
cp ${ALL_PROJECT_DIR}/deps/${ARCHITECTURE}_prebuilt/lib/libhbrt_bernoulli_aarch64.so ${RELEASE_DIR}/lib/ -rf
cp ${ALL_PROJECT_DIR}/deps/protobuf/lib/libprotobuf.so.10 ${RELEASE_DIR}/lib/ -rf
cp ${ALL_PROJECT_DIR}/deps/opencv/lib/libopencv_world.so.3.4 ${RELEASE_DIR}/lib/ -rf
cp ${ALL_PROJECT_DIR}/deps/live555/lib/libBasicUsageEnvironment.so ${RELEASE_DIR}/lib/ -rf
cp ${ALL_PROJECT_DIR}/deps/live555/lib/libgroupsock.so ${RELEASE_DIR}/lib/ -rf
cp ${ALL_PROJECT_DIR}/deps/live555/lib/libliveMedia.so ${RELEASE_DIR}/lib/ -rf
cp ${ALL_PROJECT_DIR}/deps/live555/lib/libUsageEnvironment.so ${RELEASE_DIR}/lib/ -rf
cp ${ALL_PROJECT_DIR}/deps/uWS/lib64/libuWS.so ${RELEASE_DIR}/lib/ -rf
cp ${ALL_PROJECT_DIR}/deps/xwarehouse/lib/libxwarehouse.so ${RELEASE_DIR}/lib/ -rf
cp ${ALL_PROJECT_DIR}/python_api/xstream/lib/* ${RELEASE_DIR}/lib/ -rf
cp ${ALL_PROJECT_DIR}/python_api/xproto/lib/* ${RELEASE_DIR}/lib/ -rf

# vehicle deps
cp ${ALL_PROJECT_DIR}/deps/libyuv/lib/libyuv.so ${RELEASE_DIR}/lib/ -rf
cp ${ALL_PROJECT_DIR}/deps/libjpeg-turbo/lib/libturbojpeg.so* ${RELEASE_DIR}/lib/ -rf

cp ${ALL_PROJECT_DIR}/run.sh ${RELEASE_DIR}/ -rf
cp ${ALL_PROJECT_DIR}/start_nginx.sh ${RELEASE_DIR}/ -rf
## vio configs
mkdir ${RELEASE_DIR}/configs/
if [ ${ARCHITECTURE} == "x2" ]
then
  cp ${ALL_PROJECT_DIR}/output/vioplugin/config/vio* ${RELEASE_DIR}/configs/ -rf
elif [ ${ARCHITECTURE} == "x3" ]
then
  cp ${ALL_PROJECT_DIR}/output/vioplugin/config/vio* ${RELEASE_DIR}/configs/ -rf
  echo "copy viowrapper configs"
  cp ${ALL_PROJECT_DIR}/source/common/viowrapper/config/x3dev/hb* ${RELEASE_DIR}/configs/ -rf 
  cp ${ALL_PROJECT_DIR}/output/apa ${RELEASE_DIR}/ -rf
  cp ${ALL_PROJECT_DIR}/output/apa/configs/configs/*  ${RELEASE_DIR}/configs/ -rf
  cp ${ALL_PROJECT_DIR}/output/multivioplugin/bin/multivioplugin_test ${RELEASE_DIR}/apa/ -rf
fi
cp ${ALL_PROJECT_DIR}/output/visualplugin/config/visualplugin*.json ${RELEASE_DIR}/configs/ -rf
cp ${ALL_PROJECT_DIR}/source/common/xproto/plugins/websocketplugin/configs/websocketplugin_attribute.json ${RELEASE_DIR}/configs/ -rf
## models
mkdir -p ${RELEASE_DIR}/models
cp ${ALL_PROJECT_DIR}/models/${ARCHITECTURE}/*/so/*.hbm ${RELEASE_DIR}/models/ -rf
if [ ${ARCHITECTURE} == "x3" ]
then
  cp ${ALL_PROJECT_DIR}/models/${ARCHITECTURE}/SegmentationMultitask_1024x768/so/SegmentationMultitask_1024x768.hbm ${RELEASE_DIR}/models/ -rf
  cp ${ALL_PROJECT_DIR}/models/${ARCHITECTURE}/personMultitask_1024x768/so/personMultitask_1024x768.hbm ${RELEASE_DIR}/models/personMultitask_1024x768.hbm -rf
  # apa models
  cp ${ALL_PROJECT_DIR}/source/solution_zoo/apa/models/* ${RELEASE_DIR}/models/ -rf
fi

## solutions
cp ${ALL_PROJECT_DIR}/output/face_solution ${RELEASE_DIR}/ -rf
cp ${ALL_PROJECT_DIR}/output/body_solution ${RELEASE_DIR}/ -rf
#cp ${ALL_PROJECT_DIR}/output/vehicle_solution ${RELEASE_DIR}/ -rf
cp ${ALL_PROJECT_DIR}/output/face_body_multisource ${RELEASE_DIR}/ -rf
cp ${ALL_PROJECT_DIR}/output/xwarehouse_sample ${RELEASE_DIR}/ -rf
## ssd_test
mkdir -p ${RELEASE_DIR}/ssd_test/config/vio_config
mkdir -p ${RELEASE_DIR}/ssd_test/config/bpu_config
cp ${ALL_PROJECT_DIR}/output/face_solution/configs/bpu_config.json ${RELEASE_DIR}/ssd_test/config/bpu_config
mkdir -p ${RELEASE_DIR}/ssd_test/config/models
cp -r ${ALL_PROJECT_DIR}/models/${ARCHITECTURE}/ssd/so/*  ${RELEASE_DIR}/ssd_test/config/models
cp -r ${ALL_PROJECT_DIR}/source/solution_zoo/xstream/methods/ssd_method/config/* ${RELEASE_DIR}/ssd_test/config
if [ ${ARCHITECTURE} == "x3" ]
then
cp -r ${RELEASE_DIR}/configs/hb* ${RELEASE_DIR}/ssd_test/config/vio_config
cp -r ${RELEASE_DIR}/configs/vio ${RELEASE_DIR}/ssd_test/config/vio_config
cp ${ALL_PROJECT_DIR}/output/video_box ${RELEASE_DIR}/ -rf
fi
cp -r ${ALL_PROJECT_DIR}/build/bin/ssd_method_test ${RELEASE_DIR}/ssd_test/
cp -r ${ALL_PROJECT_DIR}/source/solution_zoo/xstream/methods/ssd_method/test/data ${RELEASE_DIR}/ssd_test
## python api
mkdir -p ${RELEASE_DIR}/python_api
mkdir -p ${RELEASE_DIR}/python_api/tests
cp ${ALL_PROJECT_DIR}/python_api/xstream/xstream ${RELEASE_DIR}/python_api/ -rf
cp ${ALL_PROJECT_DIR}/python_api/xproto/xproto ${RELEASE_DIR}/python_api/ -rf
cp ${ALL_PROJECT_DIR}/python_api/xstream/tests/* ${RELEASE_DIR}/python_api/tests/ -rf
cp ${ALL_PROJECT_DIR}/python_api/xproto/tests/* ${RELEASE_DIR}/python_api/tests/ -rf
## webservice
cp ${ALL_PROJECT_DIR}/webservice ${RELEASE_DIR}/ -rf





