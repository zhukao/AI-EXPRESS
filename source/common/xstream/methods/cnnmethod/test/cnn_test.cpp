/**
 * * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: cnn_test.cpp
 * @Brief: cnnmethod unit test
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-03-05 12:27:05
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-03-05 15:18:10
 */

#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include "bpu_predict/bpu_io.h"
#include "bpu_predict/bpu_predict.h"
#include "bpu_predict/bpu_predict_extension.h"

#include "hobotlog/hobotlog.hpp"
#include "CNNMethod/CNNMethod.h"
#include "CNNMethod/util/ModelInfo.h"
#include "CNNMethod/util/util.h"
#include "CNNMethod/CNNConst.h"
#include "hobotxstream/method.h"
#include "hobotxstream/method_factory.h"
#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision_type/vision_type.hpp"
#include "horizon/vision_type/vision_type_common.h"
#include "opencv2/opencv.hpp"
#ifdef X2
#include "./vio_wrapper.h"
#endif
#ifdef X3
#include "./vio_wrapper_global.h"
#endif
#include "hobotxstream/image_tools.h"

typedef std::shared_ptr<hobot::vision::ImageFrame> ImageFramePtr;

using xstream::BaseDataPtr;
using xstream::XStreamData;

using hobot::vision::BBox;
using hobot::vision::ImageFrame;
using hobot::vision::CVImageFrame;
using hobot::vision::Landmarks;
using hobot::vision::Point;
using hobot::vision::Points;
using hobot::vision::SnapshotInfo;

namespace xstream {
namespace method_factory {
MethodPtr CreateMethod(const std::string &method_name) {
  if ("CNNMethod" == method_name) {
    return MethodPtr(new CNNMethod());
  } else {
    return MethodPtr();
  }
}
}   // namespace method_factory
}   // namespace xstream

template <typename DType>
struct LmkSnapInfo : SnapshotInfo<DType> {
  Points PointsToSnap(const Points &in) { return in; }
};

class CnnMethodParam : public xstream::InputParam {
 public:
  CnnMethodParam(const std::string &method_name, const std::string &json_config)
      : InputParam(method_name) {
    content_ = json_config;
    is_json_format_ = true;
  }
  std::string Format() override { return content_; };

 private:
  std::string content_ = "";
};

int CNNMethodForRoiInput(std::string cfg_file,
                         std::string fb_cfg,
                         std::string img_list) {
  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();
  int ret = flow->SetConfig("config_file", cfg_file.c_str());
  if (ret != 0) return -1;

  ret = flow->Init();
  if (ret != 0) return -1;
#ifdef X2
  img_info_t feed_back_info;
  HbVioFbWrapper fb_handle(fb_cfg);
  ret = fb_handle.Init();
  if (ret != 0) return -1;

  std::ifstream img_list_file(img_list);
  std::string img_path;
  std::string gt_line;
  float x1, y1, x2, y2;

  while (getline(img_list_file, gt_line)) {
    std::istringstream gt(gt_line);
    gt >> img_path;
    gt >> x1 >> y1 >> x2 >> y2;
    if (x1 < 0 || x2 >= 1920 || y1 < 0 || y2 >= 1080) continue;

    auto xstream_rois = std::make_shared<xstream::BaseDataVector>();
    xstream_rois->name_ = "face_box";
    auto xstream_roi =
        std::make_shared<xstream::XStreamData<hobot::vision::BBox>>();
    xstream_roi->value.x1 = x1;
    xstream_roi->value.y1 = y1;
    xstream_roi->value.x2 = x2;
    xstream_roi->value.y2 = y2;
    xstream_rois->datas_.push_back(xstream_roi);
    uint32_t effective_w, effective_h;
    fb_handle.GetImgInfo(img_path, &feed_back_info, &effective_w, &effective_h);

    auto py_img = std::make_shared<hobot::vision::PymImageFrame>();
    py_img->img = feed_back_info;
    auto xstream_data = std::make_shared<xstream::XStreamData<ImageFramePtr>>();
    xstream_data->value =
        std::static_pointer_cast<hobot::vision::ImageFrame>(py_img);
    xstream_data->name_ = "pyramid";

    xstream::InputDataPtr inputdata(new xstream::InputData());
    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(xstream_rois));
    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(xstream_data));

    auto out = flow->SyncPredict(inputdata);
    fb_handle.FreeImgInfo(&feed_back_info);
  }
#endif
#ifdef X3
  HbVioFbWrapperGlobal fb_handle(fb_cfg);
  ret = fb_handle.Init();
  if (ret != 0) return -1;

  std::ifstream img_list_file(img_list);
  std::string img_path;
  std::string gt_line;
  float x1, y1, x2, y2;

  while (getline(img_list_file, gt_line)) {
    std::istringstream gt(gt_line);
    gt >> img_path;
    gt >> x1 >> y1 >> x2 >> y2;
    if (x1 < 0 || x2 >= 1920 || y1 < 0 || y2 >= 1080) continue;

    auto xstream_rois = std::make_shared<xstream::BaseDataVector>();
    xstream_rois->name_ = "face_box";
    auto xstream_roi =
        std::make_shared<xstream::XStreamData<hobot::vision::BBox>>();
    xstream_roi->value.x1 = x1;
    xstream_roi->value.y1 = y1;
    xstream_roi->value.x2 = x2;
    xstream_roi->value.y2 = y2;
    xstream_rois->datas_.push_back(xstream_roi);
    uint32_t effective_w, effective_h;
    auto py_img = fb_handle.GetImgInfo(img_path, &effective_w, &effective_h);
    auto xstream_data = std::make_shared<xstream::XStreamData<ImageFramePtr>>();
    xstream_data->value =
        std::static_pointer_cast<hobot::vision::ImageFrame>(py_img);
    xstream_data->name_ = "pyramid";

    xstream::InputDataPtr inputdata(new xstream::InputData());
    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(xstream_rois));
    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(xstream_data));

    auto out = flow->SyncPredict(inputdata);

    fb_handle.FreeImgInfo(py_img);
  }
  fb_handle.DeInit();
#endif
  delete flow;
  sleep(5);  // avoid pym fb crash
  return 0;
}

int CNNMethodForLmkInput(std::string cfg_file,
                         std::string img_list) {
  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();
  int ret = flow->SetConfig("config_file", cfg_file.c_str());
  if (ret != 0) return -1;

  ret = flow->Init();
  if (ret != 0) return -1;

  std::ifstream ifs_img_list(img_list);
  if (!ifs_img_list.is_open()) {
    LOGD << "open image list file failed." << std::endl;
    return -1;
  }

  std::string gt_data;
  std::string input_image;
  Landmarks landmark;
  landmark.values.resize(5);
  while (getline(ifs_img_list, gt_data)) {
    std::istringstream gt(gt_data);
    gt >> input_image;
    for (auto &point : landmark.values) {
      gt >> point.x >> point.y;
      LOGD << "x: " << point.x << " y: " << point.y;
    }

    auto img_bgr = cv::imread(input_image);
    int width = img_bgr.cols;
    int height = img_bgr.rows;
    LOGD << "origin image size, width: " << img_bgr.cols
         << ", height: " << img_bgr.rows << std::endl;

    cv::Mat yuv_420(height * 3 / 2, width, CV_8UC1);
    cv::cvtColor(img_bgr, yuv_420, CV_BGR2YUV_I420);
    uint8_t *output_data = nullptr;
    int output_size, output_1_stride, output_2_stride;
    HobotXStreamConvertImage(yuv_420.data,
                          height * width * 3 / 2,
                          width, height,
                          width, width / 2,
                          IMAGE_TOOLS_RAW_YUV_I420,
                          IMAGE_TOOLS_RAW_YUV_NV12,
                          &output_data, &output_size,
                          &output_1_stride,
                          &output_2_stride);

    cv::Mat img_nv12 = cv::Mat(height * 3 / 2, width, CV_8UC1);
    memcpy(img_nv12.data, output_data, output_size);
    HobotXStreamFreeImage(output_data);

    auto face_img = std::make_shared<hobot::vision::CVImageFrame>();

    face_img->img = img_nv12;
    face_img->pixel_format =
        HorizonVisionPixelFormat::kHorizonVisionPixelFormatRawNV12;

    auto xstream_landmarks = std::make_shared<
                             xstream::XStreamData<Landmarks>>();
    xstream_landmarks->value = landmark;

    auto snap = std::make_shared<XStreamData<
                std::shared_ptr<LmkSnapInfo<BaseDataPtr>>>>();
    auto snap_shot_info = std::make_shared<LmkSnapInfo<BaseDataPtr>>();
    snap->value = snap_shot_info;
    snap_shot_info->userdata.resize(2);
    snap_shot_info->userdata[1] = xstream_landmarks;
    snap_shot_info->snap = face_img;

    auto p_persons = std::make_shared<xstream::BaseDataVector>();
    auto p_one_person = std::make_shared<xstream::BaseDataVector>();
    p_persons->datas_.push_back(p_one_person);
    p_persons->name_ = "snap_list";
    p_one_person->datas_.push_back(snap);

    xstream::InputDataPtr inputdata(new xstream::InputData());
    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(p_persons));
    auto out = flow->SyncPredict(inputdata);
  }
  delete flow;
  return 0;
}

int CNNMethodForDDRInput(std::string box_file,
                         std::string kps_file,
                         std::string cfg_file,
                         xstream::LmkSeqOutputType type) {
  xstream::CNNMethod cnn_method;
  cnn_method.Init(cfg_file);

  std::ifstream boxes_file(box_file);
  std::ifstream kpses_file(kps_file);
  std::string line1, line2;
  while (getline(boxes_file, line1) && getline(kpses_file, line2)) {
    std::istringstream box_vals(line1);
    std::istringstream kps_vals(line2);
    auto xstream_roi = std::make_shared<xstream::XStreamData<BBox>>();
    auto xstream_landmark = std::make_shared<xstream::XStreamData<Landmarks>>();
    if (type == xstream::LmkSeqOutputType::FALL) {
      xstream_landmark->value.values.resize(17);
    } else if (type == xstream::LmkSeqOutputType::GESTURE) {
      xstream_landmark->value.values.resize(21);
    } else {
      LOGE << "unsupported output type";
      return -1;
    }
    box_vals >> xstream_roi->value.x1 >> xstream_roi->value.y1
             >> xstream_roi->value.x2 >> xstream_roi->value.y2
             >> xstream_roi->value.score;
    for (auto &point : xstream_landmark->value.values) {
      kps_vals >> point.x >> point.y >> point.score;
    }

    auto xstream_rois = std::make_shared<xstream::BaseDataVector>();
    auto xstream_landmarks = std::make_shared<xstream::BaseDataVector>();
    auto disappeared_track_ids = std::make_shared<xstream::BaseDataVector>();
    xstream_rois->datas_.push_back(xstream_roi);
    xstream_landmarks->datas_.push_back(xstream_landmark);

    std::vector<std::vector<BaseDataPtr>> input;
    std::vector<xstream::InputParamPtr> param;
    input.resize(1);
    input[0].push_back(xstream_rois);
    input[0].push_back(xstream_landmarks);
    input[0].push_back(disappeared_track_ids);

    std::vector<std::vector<BaseDataPtr>> xstream_output =
        cnn_method.DoProcess(input, param);
    auto cnn_output = xstream_output[0];
    auto act_rets =
        std::static_pointer_cast<xstream::BaseDataVector>(cnn_output[0]);
    auto act_ret = std::static_pointer_cast<
        xstream::XStreamData<
            hobot::vision::Attribute<int>>>(act_rets->datas_[0]);
    LOGD << act_ret->value.value << ", " << act_ret->value.score;
  }

  cnn_method.Finalize();
  return 0;
}

int CNNMethodForvitest(std::string cfg_file, std::string fb_cfg,
                         std::string img_list) {
  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();
  std::cout << cfg_file << std::endl;
  int ret = flow->SetConfig("config_file", cfg_file.c_str());
  if (ret != 0) return -1;
  ret = flow->Init();
  if (ret != 0) return -1;

  SetLogLevel(HOBOT_LOG_DEBUG);
#ifdef X2
  img_info_t feed_back_info;
  HbVioFbWrapper fb_handle(fb_cfg);
  ret = fb_handle.Init();
  if (ret != 0) return -1;
  std::ifstream img_list_file(img_list);
  std::string img_path;
  std::string gt_line;
  float x1, y1, x2, y2;

  while (getline(img_list_file, gt_line)) {
    std::istringstream gt(gt_line);
    gt >> img_path;
    gt >> x1 >> y1 >> x2 >> y2;
    std::cout << "img_list = " << gt_line << std::endl;

    if (x1 < 0 || x2 >= 1920 || y1 < 0 || y2 >= 1080) continue;

    auto xstream_rois = std::make_shared<xstream::BaseDataVector>();
    xstream_rois->name_ = "body_box";
    auto xstream_roi =
        std::make_shared<xstream::XStreamData<hobot::vision::BBox>>();
    xstream_roi->value.x1 = x1;
    xstream_roi->value.y1 = y1;
    xstream_roi->value.x2 = x2;
    xstream_roi->value.y2 = y2;
    xstream_rois->datas_.push_back(xstream_roi);

    uint32_t effective_w, effective_h;
    fb_handle.GetImgInfo(img_path, &feed_back_info, &effective_w, &effective_h);

    auto py_img = std::make_shared<hobot::vision::PymImageFrame>();
    py_img->img = feed_back_info;
    auto xstream_data = std::make_shared<xstream::XStreamData<ImageFramePtr>>();
    xstream_data->value =
        std::static_pointer_cast<hobot::vision::ImageFrame>(py_img);
    xstream_data->name_ = "image";

    xstream::InputDataPtr inputdata(new xstream::InputData());
    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(xstream_rois));
    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(xstream_data));

    auto out = flow->SyncPredict(inputdata);
    std::cout << out->datas_.size() << std::endl;
    fb_handle.FreeImgInfo(&feed_back_info);
  }
#endif
#ifdef X3
  HbVioFbWrapperGlobal fb_handle(fb_cfg);
  ret = fb_handle.Init();
  if (ret != 0) return -1;
  std::ifstream img_list_file(img_list);
  std::string img_path;
  std::string gt_line;
  float x1, y1, x2, y2;

  while (getline(img_list_file, gt_line)) {
    std::istringstream gt(gt_line);
    gt >> img_path;
    gt >> x1 >> y1 >> x2 >> y2;
    std::cout << "img_list = " << gt_line << std::endl;

    if (x1 < 0 || x2 >= 1920 || y1 < 0 || y2 >= 1080) continue;

    auto xstream_rois = std::make_shared<xstream::BaseDataVector>();
    xstream_rois->name_ = "body_box";
    auto xstream_roi =
        std::make_shared<xstream::XStreamData<hobot::vision::BBox>>();
    xstream_roi->value.x1 = x1;
    xstream_roi->value.y1 = y1;
    xstream_roi->value.x2 = x2;
    xstream_roi->value.y2 = y2;
    xstream_rois->datas_.push_back(xstream_roi);

    uint32_t effective_w, effective_h;
    auto py_img = fb_handle.GetImgInfo(img_path, &effective_w, &effective_h);
    std::cout << "py_img = " << py_img << std::endl;
    auto xstream_data = std::make_shared<xstream::XStreamData<ImageFramePtr>>();
    xstream_data->value =
        std::static_pointer_cast<hobot::vision::ImageFrame>(py_img);
    xstream_data->name_ = "image";

    xstream::InputDataPtr inputdata(new xstream::InputData());
    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(xstream_rois));
    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(xstream_data));

    auto out = flow->SyncPredict(inputdata);
    std::cout << out->datas_.size() << std::endl;
    fb_handle.FreeImgInfo(py_img);
  }
#endif
  delete flow;
  return 0;
}

TEST(CNN_TEST, ModelInfo) {
  const char *model_file = "config/models/facePoseLMKs.hbm";
  BPU_MODEL_S* bpu_model;
  // load model
  {
    std::ifstream ifs(model_file, std::ios::in | std::ios::binary);
    if (!ifs) {
      HOBOT_CHECK(0) << "Open model file: " << model_file << " failed";
    }
    ifs.seekg(0, std::ios::end);
    int model_length = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    char *model_bin = new char[sizeof(char) * model_length];
    ifs.read(model_bin, model_length);
    ifs.close();
    bpu_model = new BPU_MODEL_S();
    int ret = HB_BPU_loadModel(model_bin, model_length, bpu_model);
    EXPECT_EQ(ret, 0);
    delete[] model_bin;
  }

  xstream::ModelInfo model_info;
  model_info.Init(bpu_model, 7);  // BPU_TYPE_IMG_NV12_SEPARATE

  std::cout << model_info;
}

TEST(CNN_TEST, LmkPose) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  std::string cfg_file = "./config/cnn_pose_lmk.json";
  std::string fb_cfg = "./config/vio_config/vio_onsemi0230_fb.json";
  #ifdef X2
  fb_cfg ="./config/vio_config/vio_onsemi0230_fb.json";
  #endif
  #ifdef X3_X2_VIO
  fb_cfg = "./config/vio_config/hb_vio_x3_1080_fb.json";
  #endif
  #ifdef X3_IOT_VIO
  fb_cfg = "./config/vio_config/vio/x3dev/iot_vio_x3_1080_fb.json";
  #endif
  std::string img_list = "./data/rect_cnn/pose_lmk/lmk_label.txt";

  int ret = CNNMethodForRoiInput(cfg_file, fb_cfg, img_list);
  EXPECT_EQ(ret, 0);
}

TEST(CNN_TEST, AntiSpf) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  std::string cfg_file = "./config/anti_spf.json";
  std::string fb_cfg = "./config/vio_config/vio_onsemi0230_fb.json";
  #ifdef X2
  fb_cfg ="./config/vio_config/vio_onsemi0230_fb.json";
  #endif
  #ifdef X3_X2_VIO
  fb_cfg = "./config/vio_config/hb_vio_x3_1080_fb.json";
  #endif
  #ifdef X3_IOT_VIO
  fb_cfg = "./config/vio_config/vio/x3dev/iot_vio_x3_1080_fb.json";
  #endif
  std::string img_list = "./data/rect_cnn/anti_spf/img_lst.txt";

  int ret = CNNMethodForRoiInput(cfg_file, fb_cfg, img_list);
  EXPECT_EQ(ret, 0);
}

TEST(CNN_TEST, AgeGender) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  std::string cfg_file = "./config/cnn_age_gender.json";
  std::string fb_cfg = "./config/vio_config/vio_onsemi0230_fb.json";
  #ifdef X2
  fb_cfg ="./config/vio_config/vio_onsemi0230_fb.json";
  #endif
  #ifdef X3_X2_VIO
  fb_cfg = "./config/vio_config/hb_vio_x3_1080_fb.json";
  #endif
  #ifdef X3_IOT_VIO
  fb_cfg = "./config/vio_config/vio/x3dev/iot_vio_x3_1080_fb.json";
  #endif
  std::string img_list = "./data/rect_cnn/pose_lmk/lmk_label.txt";

  int ret = CNNMethodForRoiInput(cfg_file, fb_cfg, img_list);
  EXPECT_EQ(ret, 0);
}

TEST(CNN_TEST, Mask) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  std::string cfg_file = "./config/cnn_mask.json";
  #ifdef X2
  std::string fb_cfg = "./config/vio_config/vio_onsemi0230_fb.json";
  #endif
  #ifdef X3_X2_VIO
  std::string fb_cfg = "./config/vio_config/hb_vio_x3_1080_fb.json";
  #endif
  #ifdef X3_IOT_VIO
  std::string fb_cfg = "./config/vio_config/vio/x3dev/iot_vio_x3_1080_fb.json";
  #endif
  std::string img_list = "./data/rect_cnn/pose_lmk/lmk_label.txt";

  int ret = CNNMethodForRoiInput(cfg_file, fb_cfg, img_list);
  EXPECT_EQ(ret, 0);
}

TEST(CNN_TEST, FaceQuality) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  std::string cfg_file = "./config/face_quality.json";
  std::string fb_cfg = "./config/vio_config/vio_onsemi0230_fb.json";
  #ifdef X2
  fb_cfg ="./config/vio_config/vio_onsemi0230_fb.json";
  #endif
  #ifdef X3_X2_VIO
  fb_cfg = "./config/vio_config/hb_vio_x3_1080_fb.json";
  #endif
  #ifdef X3_IOT_VIO
  fb_cfg = "./config/vio_config/vio/x3dev/iot_vio_x3_1080_fb.json";
  #endif
  std::string img_list = "./data/rect_cnn/pose_lmk/lmk_label.txt";

  int ret = CNNMethodForRoiInput(cfg_file, fb_cfg, img_list);
  EXPECT_EQ(ret, 0);
}

TEST(CNN_TEST, FaceID) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  std::string cfg_file = "./config/feature.json";
  std::string img_list = "./data/cnn_feature/feature_img_list.txt";

  int ret = CNNMethodForLmkInput(cfg_file, img_list);
  EXPECT_EQ(ret, 0);
}

TEST(CNN_TEST, VehicleColor) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  std::string cfg_file = "./config/cnn_vehicle_color.json";
  std::string fb_cfg = "./config/vio_config/vio_onsemi0230_fb.json";
  #ifdef X2
  fb_cfg ="./config/vio_config/vio_onsemi0230_fb.json";
  #endif
  #ifdef X3_X2_VIO
  fb_cfg = "./config/vio_config/hb_vio_x3_1080_fb.json";
  #endif
  #ifdef X3_IOT_VIO
  fb_cfg = "./config/vio_config/vio/x3dev/iot_vio_x3_1080_fb.json";
  #endif
  std::string img_list = "./data/rect_cnn/vehicle/img_lst.txt";

  int ret = CNNMethodForRoiInput(cfg_file, fb_cfg, img_list);
  EXPECT_EQ(ret, 0);
}

TEST(CNN_TEST, VehicleType) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  std::string cfg_file = "./config/cnn_vehicle_type.json";
  std::string fb_cfg = "./config/vio_config/vio_onsemi0230_fb.json";
  #ifdef X2
  fb_cfg ="./config/vio_config/vio_onsemi0230_fb.json";
  #endif
  #ifdef X3_X2_VIO
  fb_cfg = "./config/vio_config/hb_vio_x3_1080_fb.json";
  #endif
  #ifdef X3_IOT_VIO
  fb_cfg = "./config/vio_config/vio/x3dev/iot_vio_x3_1080_fb.json";
  #endif
  std::string img_list = "./data/rect_cnn/vehicle/img_lst.txt";

  int ret = CNNMethodForRoiInput(cfg_file, fb_cfg, img_list);
  EXPECT_EQ(ret, 0);
}

TEST(CNN_TEST, PlateNum) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  std::string cfg_file = "./config/cnn_plate_num.json";
  std::string fb_cfg = "./config/vio_config/vio_onsemi0230_fb.json";
  #ifdef X2
  fb_cfg ="./config/vio_config/vio_onsemi0230_fb.json";
  #endif
  #ifdef X3_X2_VIO
  fb_cfg = "./config/vio_config/hb_vio_x3_1080_fb.json";
  #endif
  #ifdef X3_IOT_VIO
  fb_cfg = "./config/vio_config/vio/x3dev/iot_vio_x3_1080_fb.json";
  #endif
  std::string img_list = "./data/rect_cnn/platenum/platenum.txt";

  int ret = CNNMethodForRoiInput(cfg_file, fb_cfg, img_list);
  EXPECT_EQ(ret, 0);
}

TEST(CNN_TEST, PlateNumClassify) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  std::string cfg_file = "./config/cnn_plate_num_classify.json";
  std::string fb_cfg = "./config/vio_config/vio_onsemi0230_fb.json";
  #ifdef X2
  fb_cfg ="./config/vio_config/vio_onsemi0230_fb.json";
  #endif
  #ifdef X3_X2_VIO
  fb_cfg = "./config/vio_config/hb_vio_x3_1080_fb.json";
  #endif
  #ifdef X3_IOT_VIO
  fb_cfg = "./config/vio_config/vio/x3dev/iot_vio_x3_1080_fb.json";
  #endif
  std::string img_list = "./data/rect_cnn/pose_lmk/lmk_label.txt";

  int ret = CNNMethodForRoiInput(cfg_file, fb_cfg, img_list);
  EXPECT_EQ(ret, 0);
}

TEST(DDR_TEST, FallDet) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  std::string box_file = "./data/fall_cnn/boxes.txt";
  std::string kps_file = "./data/fall_cnn/kpses.txt";
  std::string cfg_file = "./config/method_conf/fall_det.json";
  int ret = CNNMethodForDDRInput(box_file, kps_file, cfg_file,
                                 xstream::LmkSeqOutputType::FALL);
  EXPECT_EQ(ret, 0);
}

TEST(DDR_TEST, GestureRecog) {
  std::string box_file = "./data/gesture_cnn/hand_boxes.txt";
  std::string kps_file = "./data/gesture_cnn/hand_kpses.txt";
  std::string cfg_file = "./config/method_conf/gesture_det.json";
  int ret = CNNMethodForDDRInput(box_file, kps_file, cfg_file,
                                 xstream::LmkSeqOutputType::GESTURE);
  EXPECT_EQ(ret, 0);
}

TEST(CNN_TEST, UpdateConfig) {
  std::string cfg_file = "./config/cnn_plate_num.json";

  // init the SDK
  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();
  int ret = flow->SetConfig("config_file", cfg_file.c_str());
  EXPECT_EQ(ret, 0);

  ret = flow->Init();
  EXPECT_EQ(ret, 0);
  auto version = flow->GetVersion("plate_num");
  LOGD << version << std::endl;
  auto retConfig = flow->GetConfig("plate_num");
  EXPECT_EQ(retConfig->is_json_format_, true);
  auto config_content = retConfig->Format();
  LOGD << config_content << std::endl;
  auto config_content_gt =
      "{\n"
      "\t\"enable_conformance_test\" : 0\n}"
      "\n";
  xstream::InputParamPtr method_param(
      new CnnMethodParam("plate_num", config_content_gt));
  ret = flow->UpdateConfig("plate_num", method_param);
  EXPECT_EQ(ret, 0);

  delete flow;
  flow = nullptr;
}

TEST(CNN_TEST, testXBoxAction) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  std::string cfg_file = "./config/cnn_xbox_action.json";
#ifdef X2
  std::string fb_cfg = "./config/vio_config/vio_onsemi0230_fb.json";
#endif
#ifdef X3_X2_VIO
  std::string fb_cfg = "./config/vio_config/hb_vio_x3_1080_fb.json";
#endif
#ifdef X3_IOT_VIO
  std::string fb_cfg = "./config/vio_config/vio/x3dev/iot_vio_x3_1080_fb.json";
#endif
  std::string img_list = "./data/xbox_cnn/xbox_cnn_list.txt";

  int ret = CNNMethodForvitest(cfg_file, fb_cfg, img_list);
  EXPECT_EQ(ret, 0);
}

int CNNMethodForHandLmk(std::string cfg_file, std::string fb_cfg) {
  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();
  std::cout << cfg_file << std::endl;
  int ret = flow->SetConfig("config_file", cfg_file.c_str());
  if (ret != 0) return -1;
  ret = flow->Init();
  if (ret != 0) return -1;

  auto create_fake_nv12 = [&](int width, int height, cv::Mat &img_nv12) {
    img_nv12 = cv::Mat(height * 3 / 2, width, CV_8UC1);
    uint8_t *nv12 = img_nv12.ptr<uint8_t>();
    memset(nv12, 0x5, width * height * 3 / 2);
  };
  SetLogLevel(HOBOT_LOG_DEBUG);
#ifdef X2
  img_info_t feed_back_info;
  HbVioFbWrapper fb_handle(fb_cfg);
  ret = fb_handle.Init();
  if (ret != 0) return -1;

  auto xstream_rois = std::make_shared<xstream::BaseDataVector>();
  xstream_rois->name_ = "hand_box";
  auto xstream_roi =
      std::make_shared<xstream::XStreamData<hobot::vision::BBox>>();
  xstream_roi->value.x1 = 0;
  xstream_roi->value.y1 = 0;
  xstream_roi->value.x2 = 128;
  xstream_roi->value.y2 = 128;
  xstream_rois->datas_.push_back(xstream_roi);

  cv::Mat img_nv12;
  int width = 1920;
  int height = 1080;
  create_fake_nv12(width, height, img_nv12);
  fb_handle.GetImgInfo(img_nv12.data, width, height, &feed_back_info);

  auto py_img = std::make_shared<hobot::vision::PymImageFrame>();
  py_img->img = feed_back_info;
  auto xstream_data = std::make_shared<xstream::XStreamData<ImageFramePtr>>();
  xstream_data->value =
      std::static_pointer_cast<hobot::vision::ImageFrame>(py_img);
  xstream_data->name_ = "pyramid";

  xstream::InputDataPtr inputdata(new xstream::InputData());
  inputdata->datas_.push_back(
      std::static_pointer_cast<xstream::BaseData>(xstream_rois));
  inputdata->datas_.push_back(
      std::static_pointer_cast<xstream::BaseData>(xstream_data));

  auto out = flow->SyncPredict(inputdata);
  std::cout << out->datas_.size() << std::endl;
  fb_handle.FreeImgInfo(&feed_back_info);
#endif
#ifdef X3
  HbVioFbWrapperGlobal fb_handle(fb_cfg);
  ret = fb_handle.Init();
  if (ret != 0) return -1;

  auto xstream_rois = std::make_shared<xstream::BaseDataVector>();
  xstream_rois->name_ = "hand_box";
  auto xstream_roi =
      std::make_shared<xstream::XStreamData<hobot::vision::BBox>>();
  xstream_roi->value.x1 = 0;
  xstream_roi->value.y1 = 0;
  xstream_roi->value.x2 = 128;
  xstream_roi->value.y2 = 128;
  xstream_rois->datas_.push_back(xstream_roi);

  cv::Mat img_nv12;
  int width = 1920;
  int height = 1080;
  create_fake_nv12(width, height, img_nv12);
  auto py_image_frame_ptr = fb_handle.GetImgInfo(img_nv12.data, width, height);
  std::cout << "py_img = " << py_image_frame_ptr << std::endl;
  auto xstream_data = std::make_shared<xstream::XStreamData<ImageFramePtr>>();
  xstream_data->value =
      std::static_pointer_cast<hobot::vision::ImageFrame>(py_image_frame_ptr);
  xstream_data->name_ = "pyramid";

  xstream::InputDataPtr inputdata(new xstream::InputData());
  inputdata->datas_.push_back(
      std::static_pointer_cast<xstream::BaseData>(xstream_rois));
  inputdata->datas_.push_back(
      std::static_pointer_cast<xstream::BaseData>(xstream_data));

  auto out = flow->SyncPredict(inputdata);
  std::cout << out->datas_.size() << std::endl;
  fb_handle.FreeImgInfo(py_image_frame_ptr);
#endif
  delete flow;
  return 0;
}

TEST(CNN_TEST, testHandLmk) {
  std::string cfg_file = "./config/cnn_handLmk.json";
#ifdef X2
  std::string fb_cfg = "./config/vio_config/vio_onsemi0230_fb.json";
#endif
#ifdef X3_X2_VIO
  std::string fb_cfg = "./config/vio_config/hb_vio_x3_1080_fb.json";
#endif
#ifdef X3_IOT_VIO
  std::string fb_cfg = "./config/vio_config/vio/x3dev/iot_vio_x3_1080_fb.json";
#endif

  int ret = CNNMethodForHandLmk(cfg_file, fb_cfg);
  EXPECT_EQ(ret, 0);
}

