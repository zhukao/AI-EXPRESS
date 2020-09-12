//
// Created by xudong.du on 2020.04.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#include "ssd_method/ssd_method.h"

#include <gtest/gtest.h>
#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision_type/vision_type.hpp"
#include "opencv2/opencv.hpp"
#include "ssd_method/model_info.h"
#include "ssd_method/Utils.h"
#ifdef X2
#include "./vio_wrapper.h"
#endif
#ifdef X3
#include "./vio_wrapper_global.h"
#endif

using hobot::vision::BBox;
using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::InputData;
using xstream::InputDataPtr;
using xstream::InputParamPtr;
using xstream::XStreamData;

using hobot::vision::CVImageFrame;
using hobot::vision::ImageFrame;
using xstream::SSDMethod;

typedef std::shared_ptr<hobot::vision::ImageFrame> ImageFramePtr;

TEST(SSD_TEST, ModelInfo) {
  std::string model_file = "config/models/ssd.hbm";
  BPU_MODEL_S bpu_handle;
  SetLogLevel(HOBOT_LOG_INFO);
  int ret_val = LoadModel(model_file, &bpu_handle);
  ASSERT_TRUE(ret_val == 0);  // NOLINT
  PrintModelInfo(&bpu_handle);
  HB_BPU_releaseModel(&bpu_handle);
}

TEST(SSD_TEST, BoxDetect) {
  SSDMethod ssd_method;
  // 注意config文件名称
  std::string config_file = "./config/ssd_module.json";
  std::string img_list = "./data/image.list";
#ifdef X2
  std::string vio_file = "./config/vio_config/vio_onsemi0230_fb.json";
#endif
#ifdef X3_X2_VIO
  std::string vio_file = "./config/vio_config/hb_vio_x3_1080_fb.json";
#endif
#ifdef X3_IOT_VIO
  std::string vio_file =
      "./config/vio_config/vio/x3dev/iot_vio_x3_1080_fb.json";
#endif
  // SetLogLevel(HOBOT_LOG_DEBUG);
  int ret = ssd_method.Init(config_file);
  std::ifstream ifs(img_list);
  ASSERT_TRUE(ifs.is_open());
#ifdef X2
  img_info_t feed_back_info;
  HbVioFbWrapper fb_handle(vio_file);
  ret = fb_handle.Init();
  ASSERT_TRUE(ret == 0); // NOLINT

  std::string input_image;
  std::string gt_data;
  size_t box_num = 0;
  while (getline(ifs, gt_data)) {
    std::cout << gt_data << std::endl;
    std::istringstream gt(gt_data);
    gt >> input_image;
    gt >> box_num;
    uint32_t effective_w, effective_h;
    fb_handle.GetImgInfo(input_image, &feed_back_info, &effective_w,
                         &effective_h);

    auto py_img = std::make_shared<hobot::vision::PymImageFrame>();
    py_img->img = feed_back_info;
    auto xstream_img = std::make_shared<xstream::XStreamData<ImageFramePtr>>();
    xstream_img->value =
        std::static_pointer_cast<hobot::vision::ImageFrame>(py_img);
    xstream_img->name_ = "pyramid";

    std::vector<std::vector<BaseDataPtr>> input;
    std::vector<xstream::InputParamPtr> param;
    input.resize(1);
    input[0].push_back(xstream_img);
    std::vector<std::vector<BaseDataPtr>> xstream_output =
        ssd_method.DoProcess(input, param);
    fb_handle.FreeImgInfo(&feed_back_info);

    std::cout << xstream_output.size() << std::endl;
    ASSERT_TRUE(xstream_output.size() == 1); // NOLINT
    auto ssd_out = xstream_output[0];
    auto box_out = std::static_pointer_cast<BaseDataVector>(ssd_out[0]);
    for (size_t i = 0; i < box_out->datas_.size(); i++) {
      auto item =
          std::static_pointer_cast<xstream::XStreamData<hobot::vision::BBox>>(
              box_out->datas_[i]);
      std::cout << item->value << std::endl;
    }
    ASSERT_TRUE(box_out->datas_.size() == box_num); // NOLINT
  }
#endif

#if defined(X3_X2_VIO) || defined(X3_IOT_VIO)
  HbVioFbWrapperGlobal fb_handle(vio_file);
  ret = fb_handle.Init();
  ASSERT_TRUE(ret == 0); // NOLINT

  std::string input_image;
  std::string gt_data;
  size_t box_num = 0;
  while (getline(ifs, gt_data)) {
    std::cout << gt_data << std::endl;
    std::istringstream gt(gt_data);
    gt >> input_image;
    gt >> box_num;
    uint32_t effective_w, effective_h;
    auto py_img = fb_handle.GetImgInfo(input_image, &effective_w, &effective_h);
    auto xstream_img = std::make_shared<xstream::XStreamData<ImageFramePtr>>();
    xstream_img->value =
        std::static_pointer_cast<hobot::vision::ImageFrame>(py_img);
    xstream_img->name_ = "pyramid";

    std::vector<std::vector<BaseDataPtr>> input;
    std::vector<xstream::InputParamPtr> param;
    input.resize(1);
    input[0].push_back(xstream_img);
    std::vector<std::vector<BaseDataPtr>> xstream_output =
        ssd_method.DoProcess(input, param);
    fb_handle.FreeImgInfo(py_img);

    std::cout << xstream_output.size() << std::endl;
    ASSERT_TRUE(xstream_output.size() == 1); // NOLINT
    auto ssd_out = xstream_output[0];
    auto box_out = std::static_pointer_cast<BaseDataVector>(ssd_out[0]);
    for (size_t i = 0; i < box_out->datas_.size(); i++) {
      auto item =
          std::static_pointer_cast<xstream::XStreamData<hobot::vision::BBox>>(
              box_out->datas_[i]);
      std::cout << item->value << std::endl;
    }
    ASSERT_TRUE(box_out->datas_.size() == box_num); // NOLINT
  }
#endif

  ssd_method.Finalize();
}

TEST(SSD_TEST, BoxDetectWorkflow) {
  std::string config_file = "./config/ssd_test_workflow.json";
  std::string img_list = "./data/image.list";
#ifdef X2
  std::string vio_file = "./config/vio_config/vio_onsemi0230_fb.json";
#endif
#ifdef X3_X2_VIO
  std::string vio_file = "./config/vio_config/hb_vio_x3_1080_fb.json";
#endif
#ifdef X3_IOT_VIO
  std::string vio_file =
      "./config/vio_config/vio/x3dev/iot_vio_x3_1080_fb.json";
#endif

  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();
  int ret = flow->SetConfig("config_file", config_file.c_str());
  ASSERT_TRUE(ret == 0); // NOLINT

  ret = flow->Init();
  ASSERT_TRUE(ret == 0); // NOLINT
  std::ifstream ifs(img_list);
  ASSERT_TRUE(ifs.is_open());
#ifdef X2
  img_info_t feed_back_info;
  HbVioFbWrapper fb_handle(vio_file);
  ret = fb_handle.Init();
  ASSERT_TRUE(ret == 0); // NOLINT

  std::string input_image;
  std::string gt_data;
  size_t box_num = 0;
  while (getline(ifs, gt_data)) {
    std::cout << gt_data << std::endl;
    std::istringstream gt(gt_data);
    gt >> input_image;
    gt >> box_num;
    uint32_t effective_w, effective_h;
    fb_handle.GetImgInfo(input_image, &feed_back_info, &effective_w,
                         &effective_h);

    auto py_img = std::make_shared<hobot::vision::PymImageFrame>();
    py_img->img = feed_back_info;
    auto xstream_img = std::make_shared<xstream::XStreamData<ImageFramePtr>>();
    xstream_img->value =
        std::static_pointer_cast<hobot::vision::ImageFrame>(py_img);
    xstream_img->name_ = "pyramid";
    xstream::InputDataPtr inputdata(new xstream::InputData());
    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(xstream_img));
    auto out = flow->SyncPredict(inputdata);
    fb_handle.FreeImgInfo(&feed_back_info);

    ASSERT_TRUE(out->datas_.size() == 1); // NOLINT
    auto box_out =
        std::static_pointer_cast<xstream::BaseDataVector>(out->datas_[0]);
    std::cout << "output name = " << box_out->name_
              << ", data len = " << box_out->datas_.size() << std::endl;
    for (const auto item : box_out->datas_) {
      auto box =
          std::static_pointer_cast<xstream::XStreamData<hobot::vision::BBox>>(
              item);
      std::cout << box->value << std::endl;
    }
    ASSERT_TRUE(box_out->datas_.size() == box_num); // NOLINT
  }
#endif

#if defined(X3_X2_VIO) || defined(X3_IOT_VIO)
  HbVioFbWrapperGlobal fb_handle(vio_file);
  ret = fb_handle.Init();
  ASSERT_TRUE(ret == 0); // NOLINT

  std::string input_image;
  std::string gt_data;
  size_t box_num = 0;
  while (getline(ifs, gt_data)) {
    std::cout << gt_data << std::endl;
    std::istringstream gt(gt_data);
    gt >> input_image;
    gt >> box_num;
    uint32_t effective_w, effective_h;
    auto py_img = fb_handle.GetImgInfo(input_image, &effective_w, &effective_h);
    auto xstream_img = std::make_shared<xstream::XStreamData<ImageFramePtr>>();
    xstream_img->value =
        std::static_pointer_cast<hobot::vision::ImageFrame>(py_img);
    xstream_img->name_ = "pyramid";
    xstream::InputDataPtr inputdata(new xstream::InputData());
    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(xstream_img));
    auto out = flow->SyncPredict(inputdata);
    fb_handle.FreeImgInfo(py_img);

    ASSERT_TRUE(out->datas_.size() == 1); // NOLINT
    auto box_out =
        std::static_pointer_cast<xstream::BaseDataVector>(out->datas_[0]);
    std::cout << "output name = " << box_out->name_
              << ", data len = " << box_out->datas_.size() << std::endl;
    for (const auto item : box_out->datas_) {
      auto box =
          std::static_pointer_cast<xstream::XStreamData<hobot::vision::BBox>>(
              item);
      std::cout << box->value << std::endl;
    }
    ASSERT_TRUE(box_out->datas_.size() == box_num); // NOLINT
  }
#endif

  delete flow;
}
