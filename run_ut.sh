#!/usr/bin/sh

function usage() {
    echo "usage: sh run_ut.sh"
    exit 1
}

function CheckRet() {
if [ $? -ne 0 ] ; then
    echo "failed to "$1
    exit 1
fi
}

function RunXProto(){
  ${UT_DIR}/xproto/unit_test 
  # ${UT_DIR}/xproto/vioplugin_test
  # rm configs -rf
}

function RunXProtoPlugin(){
  ${UT_DIR}/xproto_plugin/smartplugin_test 
}

function RunVisionType(){
  ${UT_DIR}/vision_type/gtest_vision_type
}

function RunXStream(){
  cd xstream
  ./xstream_test
  #${UT_DIR}/xstream/xstream_multisource_test
  ./xstream_threadmodel_test
  ./xstream_threadorder_test
  ./xstream_threadsafe_test
  ./xstream_unit_test
  ./config_test
  ./cpp_api_test
  ./node_test
  ./workflow_test
  # ./timer_test
  # ./method_init_test
  # ./disable_method_test
  # ./profiler_test
  # rm test -rf
  # rm config -rf
  cd ../
}

function RunCNNMethod(){
  cd cnn_method
  export LD_LIBRARY_PATH=../../lib/
  ./CNNMethod_unit_test
  # rm config -rf
  # rm data -rf
  cd ..
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

function RunFasterrcnnMethod(){
 ${UT_DIR}/fasterrcnn_method/gtest_fasterrcnn
}

function RunGrading(){
  cd grading_method
  ./gtest_grading
  ./grading_example
  cd ..
}

function RunMerge(){
  cd merge_method
  export LD_LIBRARY_PATH=../../lib/
  ./gtest_head_face
  cd ..
}

function RunMot(){
  cd mot_method
  ./gtest_mot
  cd ..
}

function RunSnapshot(){
  cd snapshot_method
  export LD_LIBRARY_PATH=../../lib/
  ./gtest_snapshot
  cd ..
}

function RunVoteMehtod(){
  ${UT_DIR}/vote_method/vote_method_test
}

function ChangeRunMode(){
  sed -i "s#\(\./face_solution/configs/face_solution.json ./configs/visualplugin_face.json -i\).*#\1 ${1}#g" run.sh
  sed -i "s#\(\./face_solution/configs/face_recog_solution.json ./configs/visualplugin_face.json -i\).*#\1 ${1}#g" run.sh
  sed -i "s#\(\./body_solution/configs/body_solution.json ./configs/visualplugin_body.json -i\).*#\1 ${1}#g" run.sh
  sed -i "s#\(\./body_solution/configs/xbox_solution.json ./configs/visualplugin_body.json -i\).*#\1 ${1}#g" run.sh
  sed -i "s#\(\./body_solution/configs/behavior_solution.json ./configs/visualplugin_body.json -i\).*#\1 ${1}#g" run.sh
  sed -i "s#\(\./body_solution/configs/guesture_solution.json ./configs/visualplugin_body.json -i\).*#\1 ${1}#g" run.sh
  sed -i "s#\(\./video_box/configs/body_solution.json  ./configs/visualplugin_body.json -i\).*#\1 ${1}#g" run.sh
  sed -i "s#\(\./body_solution/configs/dance_solution.json ./configs/visualplugin_body.json -i\).*#\1 ${1}#g" run.sh
  sed -i "s#\(\${face_body_multisource_vio} ./face_body_multisource/configs/face_body_solution.json -i\).*#\1 ${1}#g" run.sh
}


function RunSolutions(){
  cd ..
 
  ARCHITECTURE=${1}
  if [ ${ARCHITECTURE} == "x3" ];then
    ChangeRunMode ut
    sh run.sh face x3dev hg cache
    sh run.sh face x3dev hg jpg
    sh run.sh face x3dev hg nv12
    sh run.sh face_recog x3dev hg cache
    sh run.sh face_recog x3dev hg jpg
    sh run.sh face_recog x3dev hg nv12
    sh run.sh body x3dev hg cache
    sh run.sh body x3dev hg jpg
    sh run.sh body x3dev hg nv12
    sh run.sh xbox x3dev hg cache
    sh run.sh xbox x3dev hg jpg
    sh run.sh xbox x3dev hg nv12
    sh run.sh behavior x3dev hg cache
    sh run.sh behavior x3dev hg jpg
    sh run.sh behavior x3dev hg nv12
    sh run.sh video_box x3dev hg cache
    sh run.sh video_box x3dev hg jpg
    sh run.sh video_box x3dev hg nv12
    sh run.sh gesture x3dev hg cache
    sh run.sh gesture x3dev hg jpg
    sh run.sh gesture x3dev hg nv12
    sh run.sh tv_dance x3dev hg cache
    sh run.sh tv_dance x3dev hg jpg
    sh run.sh tv_dance x3dev hg nv12
    sh run.sh face_body_multisource x3dev hg cache
    sh run.sh face_body_multisource x3dev hg jpg
    sh run.sh face_body_multisource x3dev hg nv12
    ChangeRunMode normal
  elif [ ${ARCHITECTURE} == "x2" ];then
    ChangeRunMode ut
    sh run.sh face 96board
    sh run.sh face_recog 96board
    sh run.sh body 96board
    sh run.sh xbox 96board
    sh run.sh behavior 96board
    # sh run.sh video_box 96board  # No support plan for the x2 platform
    sh run.sh gesture 96board
    sh run.sh tv_dance 96board
    sh run.sh face_body_multisource 96board
    # sh run.sh face 96board hg cache # Cannot run
    # sh run.sh face 96board hg jpg  # Cannot run
    # sh run.sh face 96board hg nv12 # Cannot run
    ChangeRunMode normal
  fi

  cd unit_test
}

function RunSSDMethod(){
  cd ssd_method
  export LD_LIBRARY_PATH=../../lib/
  ./ssd_method_test
  cd ..
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

#########test succ begin#################
set -eux

export LD_LIBRARY_PATH=../lib/
UT_DIR=.
ARCHITECTURE=$(cat platform.tmp)

RunXProto
RunXProtoPlugin
RunVisionType
RunXStream
RunGrading
RunSnapshot
RunMerge
RunMot
RunSSDMethod ${ARCHITECTURE}

RunSolutions ${ARCHITECTURE}
#########test succ end##################

# RunCNNMethod ${ARCHITECTURE}
# RunCNNMultistageMethod ${ARCHITECTURE}
# RunFasterrcnnMethod ${ARCHITECTURE}
# RunVoteMehtod



