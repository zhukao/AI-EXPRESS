//
// Created by yaoyao.sun on 2019-05-14.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#include <gtest/gtest.h>

#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>

#include "yuv_utils.h"
#include "FasterRCNNMethod/dump.h"

#include "bpu_predict/bpu_io.h"
#include "horizon/vision_type/vision_type.hpp"

#include "hobotxsdk/xstream_sdk.h"
#include "opencv2/opencv.hpp"

#include "FasterRCNNMethod.h"
#include "bpu_predict/bpu_predict.h"
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

TEST(FACE_DET_TEST, Basic) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  FasterRCNNMethod faster_rcnn_method;
  std::string config_file = "./configs/multitask_config.json";
  faster_rcnn_method.Init(config_file);

  std::string img_list = "./test/data/image.list";
  std::ifstream ifs(img_list);
  ASSERT_TRUE(ifs.is_open());

  std::string input_image;
  while (getline(ifs, input_image)) {
    auto img_bgr = cv::imread(input_image);
    std::cout << "origin image size, width: " << img_bgr.cols
              << ", height: " << img_bgr.rows << std::endl;
    auto cv_image_frame_ptr = std::make_shared<CVImageFrame>();

    cv_image_frame_ptr->img = img_bgr;
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
    ASSERT_TRUE(xstream_output.size() == 1);    // NOLINT
    auto faster_rcnn_out = xstream_output[0];
    auto rects = std::static_pointer_cast<BaseDataVector>(faster_rcnn_out[0]);
    ASSERT_TRUE(rects->datas_.size() == 3);     // NOLINT
  }

  faster_rcnn_method.Finalize();
}
