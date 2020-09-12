#!/usr/bin/sh

function usage() {
    echo "usage: bash deploy_ut.sh"
    exit 1
}

function CheckRet() {
if [ $? -ne 0 ] ; then
    echo "failed to "$1
    exit 1
fi
}

function CopyXProto(){
  mkdir -p ${UT_DIR}/xproto
  XPROTO_UT_DIR=${UT_DIR}/xproto
  cp ${ALL_PROJECT_DIR}/source/common/xproto/plugins/vioplugin/configs/ ${XPROTO_UT_DIR} -rf
  cp ${ALL_PROJECT_DIR}/build/bin/unit_test ${XPROTO_UT_DIR} -rf
  # To check
  # cp ${ALL_PROJECT_DIR}/build/bin/vioplugin_test ${XPROTO_UT_DIR} -rf
}

function CopyXProtoPlugin(){
  mkdir -p ${UT_DIR}/xproto_plugin
  cp ${ALL_PROJECT_DIR}/build/bin/smartplugin_test ${UT_DIR}/xproto_plugin -rf
}

function CopyVisionType(){
  mkdir -p ${UT_DIR}/vision_type
  cp ${ALL_PROJECT_DIR}/build/bin/gtest_vision_type ${UT_DIR}/vision_type -rf
}

function CopyXStream(){
  mkdir -p ${UT_DIR}/xstream
  mkdir -p ${UT_DIR}/xstream/test
  XSTREAM_DIR=${UT_DIR}/xstream
  cp ${ALL_PROJECT_DIR}/source/common/xstream/framework/test/configs ${XSTREAM_DIR}/test -rf
  cp ${ALL_PROJECT_DIR}/source/common/xstream/framework/test/configs_subworkflow ${XSTREAM_DIR}/test -rf
  cp ${ALL_PROJECT_DIR}/build/bin/xstream_test ${XSTREAM_DIR} -rf
  # cp ${ALL_PROJECT_DIR}/build/bin/xstream_multisource_test ${XSTREAM_DIR} -rf
  cp ${ALL_PROJECT_DIR}/build/bin/xstream_threadmodel_test ${XSTREAM_DIR} -rf
  cp ${ALL_PROJECT_DIR}/build/bin/xstream_threadorder_test ${XSTREAM_DIR} -rf
  cp ${ALL_PROJECT_DIR}/build/bin/xstream_threadsafe_test ${XSTREAM_DIR} -rf
  cp ${ALL_PROJECT_DIR}/build/bin/xstream_unit_test ${XSTREAM_DIR} -rf
  cp ${ALL_PROJECT_DIR}/build/bin/config_test ${XSTREAM_DIR} -rf
  cp ${ALL_PROJECT_DIR}/build/bin/cpp_api_test ${XSTREAM_DIR} -rf
  cp ${ALL_PROJECT_DIR}/build/bin/node_test ${XSTREAM_DIR} -rf
  cp ${ALL_PROJECT_DIR}/build/bin/workflow_test ${XSTREAM_DIR} -rf

  # cp ${ALL_PROJECT_DIR}/build/bin/timer_test ${XSTREAM_DIR} -rf
  # cp ${ALL_PROJECT_DIR}/build/bin/method_init_test ${XSTREAM_DIR} -rf
  cp ${ALL_PROJECT_DIR}/source/common/xstream/framework/config ${XSTREAM_DIR} -rf
  cp ${ALL_PROJECT_DIR}/build/bin/disable_method_test ${XSTREAM_DIR} -rf
  cp ${ALL_PROJECT_DIR}/build/bin/profiler_test ${XSTREAM_DIR} -rf
}

function CopyCNNMethod(){
  mkdir -p ${UT_DIR}/cnn_method
  mkdir -p ${UT_DIR}/cnn_method/config/models
  mkdir -p ${UT_DIR}/cnn_method/config/bpu_config
  mkdir -p ${UT_DIR}/cnn_method/config/configs
  mkdir -p ${UT_DIR}/cnn_method/config/vio_config
  cp source/common/xstream/methods/cnnmethod/example/config/*.json ${UT_DIR}/cnn_method/config/ -rf
  cp source/common/xstream/methods/cnnmethod/example/config/method_conf ${UT_DIR}/cnn_method/config/ -rf
  ln -s deploy/models ${UT_DIR}/cnn_method/config/models/
  cp  source/common/xstream/methods/cnnmethod/example/data ${UT_DIR}/cnn_method -rf
  if [ ${1} == "x2" ]
  then
    cp deploy/configs/vio/* ${UT_DIR}/cnn_method/config/vio_config -rf
  elif [ ${1} == "x3" ]
  then
    cp source/common/viowrapper/config ${UT_DIR}/cnn_method/config/vio_config -rf
    cp deploy/configs/* ${UT_DIR}/cnn_method/config/vio_config -rf
  else
    echo "${1} architecture is unknown"
    exit 1
  fi
  cp deps/bpu_predict/${ARCHITECTURE}/config/bpu_config.json ${UT_DIR}/cnn_method/config/bpu_config/ -rf
  cp deps/bpu_predict/${ARCHITECTURE}/config/bpu_config.json ${UT_DIR}/cnn_method/config/configs/ -rf
  cp build/bin/CNNMethod_unit_test ${UT_DIR}/cnn_method/ -rf
}

function RunCNNMultistageMethod(){
rm config -rf
mkdir -p config/models
mkdir -p config/configs
mkdir -p config/vio_config
cp xsdk/common/xstream/methods/cnnmethod_multistage/example/config/* ./config/ -rf
ln  models/download_model/${ARCHITECTURE}/*/so/*.hbm config/models/
cp  xsdk/common/xstream/methods/cnnmethod/example/data ./ -rf
cp common/bpu-predict/config/bpu_config.json ./config/configs -rf
if [ ${1} == "x2" ]
then
  cp deploy_dev/configs/vio/vio_onsemi0230_fb.json ./config/vio_config -rf
  VIO_CONFIG_FILE=./config/vio_config/vio_onsemi0230_fb.json
elif [ ${1} == "x3" ]
then
  if [ ${2} == "VIO_NORMAL" ]
  then
    cp deploy_dev/configs/hb_vio_x3_1080_fb.json ./config/vio_config -rf
    VIO_CONFIG_FILE=./config/vio_config/hb_vio_x3_1080_fb.json
  elif [ ${2} == "VIO_HISI" ]
  then
    cp deploy_dev/configs/* ./config/vio_config -rf
    VIO_CONFIG_FILE=./config/vio_config/vio/x3dev/iot_vio_x3_1080_fb.json
  else
    echo "${2} vio interface is unknown"
    exit 1
  fi
else
  echo "${1} architecture is unknown"
  exit 1
fi
#pose_lmk
./build/bin/CNNMethod_Multistage_example do_fb_rect_cnn pose_lmk config/rect_pose_lmk.json ${VIO_CONFIG_FILE} data/rect_cnn/pose_lmk/lmk_label.txt data/rect_cnn/pose_lmk/cnn_out.txt
#antispf
./build/bin/CNNMethod_Multistage_example do_fb_rect_cnn anti_spf config/img_antispf.json ${VIO_CONFIG_FILE} data/rect_cnn/pose_lmk/lmk_label.txt data/rect_cnn/pose_lmk/cnn_out.txt
#age_gender
./build/bin/CNNMethod_Multistage_example do_fb_rect_cnn age_gender config/rect_age_gender.json ${VIO_CONFIG_FILE} data/rect_cnn/pose_lmk/lmk_label.txt data/rect_cnn/age_gender/cnn_out.txt
#face_quality
./build/bin/CNNMethod_Multistage_example do_fb_rect_cnn face_quality config/img_facequality.json ${VIO_CONFIG_FILE} data/rect_cnn/pose_lmk/lmk_label.txt data/rect_cnn/face_quality/cnn_out.txt
#face_feature
./build/bin/CNNMethod_Multistage_example do_fb_feature config/lmk_faceId.json data/cnn_feature/feature_img_list.txt data/cnn_feature/feature_out.txt

CheckRet "run cnn_multistage test"
rm config -rf
rm data -rf
}

function CopyFasterrcnnMethod(){
  mkdir -p ${UT_DIR}/fasterrcnn_method
  # mkdir -p ${UT_DIR}/fasterrcnn_method/models
  FASTERRCNN_DIR=${UT_DIR}/fasterrcnn_method
  ARCHITECTURE=${1}
  cp models/${ARCHITECTURE}/multitask/so/multitask.hbm ${FASTERRCNN_DIR}/models/ -rf
  cp models/${ARCHITECTURE}/personMultitask/so/personMultitask.hbm ${FASTERRCNN_DIR}/models/ -rf
  cp models/${ARCHITECTURE}/vehicleSolutionModels/so/vehicle_multitask.hbm ${FASTERRCNN_DIR}/models/ -rf
  cp source/common/xstream/methods/fasterrcnnmethod/test/ ${FASTERRCNN_DIR} -rf
  cp source/common/xstream/methods/fasterrcnnmethod/configs/ ${FASTERRCNN_DIR} -rf
  cp deps/bpu_predict/${ARCHITECTURE}/config/bpu_config.json ${FASTERRCNN_DIR}/configs/ -rf
  cp source/common/xproto/plugins/vioplugin/configs/vio/vio_onsemi0230_fb.json ${FASTERRCNN_DIR}/configs/ -rf
  if [ ${1} == "x2" ]
  then
    cp source/common/xproto/plugins/vioplugin/configs/vio/vio_onsemi0230_fb.json ${FASTERRCNN_DIR}/configs/ -rf
  elif [ ${1} == "x3" ]
  then
    cp source/common/viowrapper/config ${FASTERRCNN_DIR}/configs/vio_config -rf
    cp deploy/configs/* ${FASTERRCNN_DIR}/configs/vio_config -rf
  else
    echo "${1} architecture is unknown"
    exit 1
  fi
  cp ${ALL_PROJECT_DIR}/build/bin/gtest_fasterrcnn ${FASTERRCNN_DIR}
}

function CopyGrading(){
  mkdir -p ${UT_DIR}/grading_method
  GRADING_DIR=${UT_DIR}/grading_method
  cp source/solution_zoo/xstream/methods/gradingmethod/config ${GRADING_DIR} -rf
  cp ${ALL_PROJECT_DIR}/build/bin/gtest_grading ${GRADING_DIR} -rf
  cp ${ALL_PROJECT_DIR}/build/bin/grading_example ${GRADING_DIR} -rf
}

function CopyMerge(){
  mkdir -p ${UT_DIR}/merge_method
  MERGE_DIR=${UT_DIR}/merge_method
  cp source/solution_zoo/xstream/methods/mergemethod/config ${MERGE_DIR} -rf
  cp ${ALL_PROJECT_DIR}/build/bin/gtest_head_face ${MERGE_DIR} -rf
}

function CopyMot(){
  mkdir -p ${UT_DIR}/mot_method
  MOT_DIR=${UT_DIR}/mot_method
  cp source/common/xstream/methods/motmethod/config ${MOT_DIR} -rf
  cp source/common/xstream/methods/motmethod/test ${MOT_DIR} -rf
  cp ${ALL_PROJECT_DIR}/build/bin/gtest_mot ${MOT_DIR} -rf
}

function CopySnapshot(){
  mkdir -p ${UT_DIR}/snapshot_method
  SNAPSHORT_DIR=${UT_DIR}/snapshot_method
  mkdir -p ${SNAPSHORT_DIR}/test
  cp source/solution_zoo/xstream/methods/snapshotmethod/config ${SNAPSHORT_DIR} -rf
  cp source/solution_zoo/xstream/methods/snapshotmethod/test/files ${SNAPSHORT_DIR}/test/ -rf
  cp ${ALL_PROJECT_DIR}/build/bin/gtest_snapshot ${SNAPSHORT_DIR} -rf
}

function CopyVoteMehtod(){
  mkdir -p ${UT_DIR}/vote_method
  VOTE_METHOD=${UT_DIR}/vote_method
  cp source/solution_zoo/xstream/methods/vote_method/config ${VOTE_METHOD} -rf
  cp ${ALL_PROJECT_DIR}/build/bin/vote_method_test ${VOTE_METHOD} -rf
}

function CopySSDMethod(){
  mkdir -p ${UT_DIR}/ssd_method
  SSD_METHOD=${UT_DIR}/ssd_method
  mkdir -p ${SSD_METHOD}/config/models
  mkdir -p ${SSD_METHOD}/config/bpu_config
  mkdir -p ${SSD_METHOD}/config/vio_config
  cp source/solution_zoo/xstream/methods/ssd_method/config/*.json ${SSD_METHOD}/config/ -rf
  ln  models/${ARCHITECTURE}/*/so/*.hbm ${SSD_METHOD}/config/models/
  cp  source/solution_zoo/xstream/methods/ssd_method/test/data ${SSD_METHOD} -rf
  if [ ${1} == "x2" ]
  then
    cp deploy/configs/vio/* ${SSD_METHOD}/config/vio_config -rf
  elif [ ${1} == "x3" ]
  then
    cp deploy/configs/* ${SSD_METHOD}/config/vio_config -rf
  else
    echo "${1} architecture is unknown"
    exit 1
  fi
  cp ${ALL_PROJECT_DIR}/deps/bpu_predict/${1}/config/bpu_config.json ${SSD_METHOD}/config/bpu_config/ -rf
  cp ${ALL_PROJECT_DIR}/build/bin/ssd_method_test ${SSD_METHOD} -rf
}

# ARCHITECTURE="x2"
# if [ $# -ge 2 ]
# then
#   if [ ${1} == "x2" ]
#   then
#     ARCHITECTURE="x2"
#   elif [ ${1} == "x3" ]
#   then
#     ARCHITECTURE="x3"
#   else
#     usage
#   fi
# else
#   usage
# fi

# VIOINTERFACE="VIO_HISI"
# if [ $# -ge 3 ]
# then
#   if [ ${3} == "VIO_NORMAL" ]
#   then
#     VIOINTERFACE="VIO_NORMAL"
#   elif [ ${3} == "VIO_HISI" ]
#   then
#     VIOINTERFACE="VIO_HISI"
#   else
#     usage
#   fi
# fi

set -eux
bash deploy.sh
mkdir -p deploy/unit_test
ARCHITECTURE=$(cat platform.tmp)
ALL_PROJECT_DIR=$PWD
UT_DIR=${ALL_PROJECT_DIR}/deploy/unit_test

CopyXProto
CopyXProtoPlugin
CopyVisionType
CopyXStream
CopyGrading
CopySnapshot
CopyMerge
CopyMot
CopySSDMethod ${ARCHITECTURE}

# CopyCNNMethod ${ARCHITECTURE}
# RunCNNMultistageMethod ${ARCHITECTURE}
# CopyFasterrcnnMethod ${ARCHITECTURE}
# CopyVoteMehtod

cp ${ALL_PROJECT_DIR}/run_ut.sh ${UT_DIR}/ -rf
cp ${ALL_PROJECT_DIR}/platform.tmp ${UT_DIR}/ -rf