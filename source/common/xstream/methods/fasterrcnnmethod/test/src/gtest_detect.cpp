//
// Created by yaoyao.sun on 2019-05-14.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#include <gtest/gtest.h>

#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <memory>
#include <vector>

#include "yuv_utils.h"    // NOLINT
#include "FasterRCNNMethod/dump.h"

#include "bpu_predict/bpu_io.h"
#include "horizon/vision_type/vision_type.hpp"

#include "hobotxsdk/xstream_sdk.h"
#include "opencv2/opencv.hpp"

#include "FasterRCNNMethod.h"
#include "bpu_predict/bpu_predict.h"

#ifdef X2
#include "./vio_wrapper.h"
#endif
#ifdef X3
#include "./vio_wrapper_global.h"
#endif
#include "hobotlog/hobotlog.hpp"

using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::InputData;
using xstream::InputDataPtr;
using xstream::XStreamData;
using xstream::InputParamPtr;

using xstream::FasterRCNNMethod;
using hobot::vision::ImageFrame;
using hobot::vision::CVImageFrame;
using hobot::vision::PymImageFrame;
using hobot::vision::Landmarks;

TEST(FasterRCNNTest, FbPyramid) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  FasterRCNNMethod faster_rcnn_method;
  std::string config_file = "./configs/multitask_config.json";
  faster_rcnn_method.Init(config_file);

  std::string test_img = "./test/data/1080p.jpg";
  std::ifstream ifs(test_img);
  ASSERT_TRUE(ifs.is_open());
#ifdef X3_X2_VIO
  std::string fb_cfg = "./configs/vio_config/x3dev/hb_vio_x3_1080_fb.json";
#endif
#ifdef X3_IOT_VIO
  std::string fb_cfg = "./configs/vio_config/vio/x3dev/iot_vio_x3_1080_fb.json";
#endif
#ifdef X2
  img_info_t feed_back_info;
  HbVioFbWrapper fb_handle("./configs/vio_onsemi0230_fb.json");
  int ret = fb_handle.Init();
  ASSERT_TRUE(ret == 0);  // NOLINT
  uint32_t effective_w, effective_h;
  fb_handle.GetImgInfo(test_img, &feed_back_info, &effective_w, &effective_h);

  auto pym_image_frame_ptr = std::make_shared<PymImageFrame>();
  pym_image_frame_ptr->img = feed_back_info;

  auto xstream_img =
    std::make_shared<XStreamData<std::shared_ptr<ImageFrame>>>();
  xstream_img->value = pym_image_frame_ptr;

  std::vector<std::vector<BaseDataPtr>> input;
  std::vector<xstream::InputParamPtr> param;
  input.resize(1);
  input[0].push_back(xstream_img);
  std::vector<std::vector<BaseDataPtr>> xstream_output =
    faster_rcnn_method.DoProcess(input, param);
  ASSERT_EQ(xstream_output.size(), static_cast<std::size_t>(1));
  auto faster_rcnn_out = xstream_output[0];
  ASSERT_EQ(faster_rcnn_out.size(), static_cast<std::size_t>(6));
  auto face_rects =
    std::static_pointer_cast<BaseDataVector>(faster_rcnn_out[0]);
  ASSERT_EQ(face_rects->datas_.size(), static_cast<std::size_t>(3));
  auto masks = std::static_pointer_cast<BaseDataVector>(faster_rcnn_out[4]);
  ASSERT_EQ(masks->datas_.size(), static_cast<std::size_t>(3));
  auto reids = std::static_pointer_cast<BaseDataVector>(faster_rcnn_out[5]);
  ASSERT_EQ(reids->datas_.size(), static_cast<std::size_t>(3));

  faster_rcnn_method.Finalize();
  fb_handle.FreeImgInfo(&feed_back_info);
#endif
#ifdef X3
  HbVioFbWrapperGlobal fb_handle(fb_cfg);
  int ret = fb_handle.Init();
  ASSERT_TRUE(ret == 0); // NOLINT
  uint32_t effective_w, effective_h;
  auto pym_image_frame_ptr =
      fb_handle.GetImgInfo(test_img, &effective_w, &effective_h);

  auto xstream_img =
      std::make_shared<XStreamData<std::shared_ptr<ImageFrame>>>();
  xstream_img->value = pym_image_frame_ptr;

  std::vector<std::vector<BaseDataPtr>> input;
  std::vector<xstream::InputParamPtr> param;
  input.resize(1);
  input[0].push_back(xstream_img);
  std::vector<std::vector<BaseDataPtr>> xstream_output =
    faster_rcnn_method.DoProcess(input, param);
  ASSERT_EQ(xstream_output.size(), static_cast<std::size_t>(1));
  auto faster_rcnn_out = xstream_output[0];
  ASSERT_EQ(faster_rcnn_out.size(), static_cast<std::size_t>(6));
  auto face_rects =
    std::static_pointer_cast<BaseDataVector>(faster_rcnn_out[0]);
  ASSERT_EQ(face_rects->datas_.size(), static_cast<std::size_t>(3));
  auto masks = std::static_pointer_cast<BaseDataVector>(faster_rcnn_out[4]);
  ASSERT_EQ(masks->datas_.size(), static_cast<std::size_t>(3));
  auto reids = std::static_pointer_cast<BaseDataVector>(faster_rcnn_out[5]);
  ASSERT_EQ(reids->datas_.size(), static_cast<std::size_t>(3));

  faster_rcnn_method.Finalize();
  fb_handle.FreeImgInfo(pym_image_frame_ptr);
#endif
}

TEST(FasterRCNNTest, Multitask) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  FasterRCNNMethod faster_rcnn_method;
  std::string config_file = "./configs/body_multitask_config.json";
  faster_rcnn_method.Init(config_file);

  std::string test_img = "./test/data/body.jpg";
  std::ifstream ifs(test_img);
  ASSERT_TRUE(ifs.is_open());

  auto bgr_img = cv::imread(test_img);
  std::cout << "input image size, width: " << bgr_img.cols
            << ", height: " << bgr_img.rows << std::endl;

  auto cv_image_frame_ptr = std::make_shared<CVImageFrame>();
  cv_image_frame_ptr->img = bgr_img;
  cv_image_frame_ptr->pixel_format =
    HorizonVisionPixelFormat::kHorizonVisionPixelFormatRawBGR;

  auto xstream_img =
    std::make_shared<XStreamData<std::shared_ptr<ImageFrame>>>();
  xstream_img->value = cv_image_frame_ptr;

  std::vector<std::vector<BaseDataPtr>> input;
  std::vector<xstream::InputParamPtr> param;
  input.resize(1);
  input[0].push_back(xstream_img);
  std::vector<std::vector<BaseDataPtr>> xstream_output =
    faster_rcnn_method.DoProcess(input, param);
  ASSERT_EQ(xstream_output.size(), static_cast<std::size_t>(1));
  auto faster_rcnn_out = xstream_output[0];
  ASSERT_EQ(faster_rcnn_out.size(), static_cast<std::size_t>(6));
  auto face_rects =
    std::static_pointer_cast<BaseDataVector>(faster_rcnn_out[2]);
  ASSERT_EQ(face_rects->datas_.size(), static_cast<std::size_t>(1));
  auto lmks = std::static_pointer_cast<BaseDataVector>(faster_rcnn_out[3]);
  // auto lmk = std::reinterprate_cast<Landmarks *>(&(lmks->datas[0]));
  ASSERT_EQ(lmks->datas_.size(), static_cast<std::size_t>(1));
  auto poses = std::static_pointer_cast<BaseDataVector>(faster_rcnn_out[4]);
  ASSERT_EQ(poses->datas_.size(), static_cast<std::size_t>(1));
  auto kps = std::static_pointer_cast<BaseDataVector>(faster_rcnn_out[5]);
  ASSERT_EQ(kps->datas_.size(), static_cast<std::size_t>(1));

  faster_rcnn_method.Finalize();
}

TEST(FasterRCNNTest, Vehicle) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  FasterRCNNMethod faster_rcnn_method;
  std::string config_file = "./configs/vehicle_config.json";
  faster_rcnn_method.Init(config_file);

  std::string test_img = "./test/data/vehicle.jpg";
  std::ifstream ifs(test_img);
  ASSERT_TRUE(ifs.is_open());

  auto bgr_img = cv::imread(test_img);
  std::cout << "input image size, width: " << bgr_img.cols
            << ", height: " << bgr_img.rows << std::endl;

  auto cv_image_frame_ptr = std::make_shared<CVImageFrame>();
  cv_image_frame_ptr->img = bgr_img;
  cv_image_frame_ptr->pixel_format =
    HorizonVisionPixelFormat::kHorizonVisionPixelFormatRawBGR;

  auto xstream_img =
    std::make_shared<XStreamData<std::shared_ptr<ImageFrame>>>();
  xstream_img->value = cv_image_frame_ptr;

  std::vector<std::vector<BaseDataPtr>> input;
  std::vector<xstream::InputParamPtr> param;
  input.resize(1);
  input[0].push_back(xstream_img);
  std::vector<std::vector<BaseDataPtr>> xstream_output =
    faster_rcnn_method.DoProcess(input, param);
  ASSERT_EQ(xstream_output.size(), static_cast<std::size_t>(1));
  auto faster_rcnn_out = xstream_output[0];
  ASSERT_EQ(faster_rcnn_out.size(), static_cast<std::size_t>(7));
  auto plate_color =
    std::static_pointer_cast<BaseDataVector>(faster_rcnn_out[5]);
  ASSERT_EQ(plate_color->datas_.size(), static_cast<std::size_t>(1));
  auto plate_row = std::static_pointer_cast<BaseDataVector>(faster_rcnn_out[6]);
  ASSERT_EQ(plate_row->datas_.size(), static_cast<std::size_t>(1));

  faster_rcnn_method.Finalize();
}
