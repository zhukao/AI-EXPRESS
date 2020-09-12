//
// Created by shiyu.fu on 2020-04-08.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#include <gtest/gtest.h>

#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <memory>

#include "yuv_utils.h"    // NOLINT
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

TEST(FasterRCNNTest, ModelInfo) {
  std::string model_file_path = "./models/multitask.hbm";
  const char *bpu_config_path = "./configs/bpu_config.json";
  BPUHandle bpu_handle;
  // load model
  int ret = BPU_loadModel(model_file_path.c_str(),
                          &bpu_handle, bpu_config_path);
  LOGI << "Load BPU model: " << BPU_getLastError(bpu_handle);
  ASSERT_EQ(ret, 0);
  // get bpu version
  const char* version = BPU_getVersion(bpu_handle);
  LOGI << "Get BPU version: " << BPU_getLastError(bpu_handle);
  ASSERT_TRUE(version != nullptr);
  // get model names
  const char** name_list;
  int name_cnt;
  ret = BPU_getModelNameList(bpu_handle, &name_list, &name_cnt);
  LOGI << "Get name list: " << BPU_getLastError(bpu_handle);
  ASSERT_EQ(ret, 0);
  // get input info
  const char* model_name = "multitask";
  BPUModelInfo input_info;
  ret = BPU_getModelInputInfo(bpu_handle, model_name, &input_info);
  LOGI << "Get model input info: " << BPU_getLastError(bpu_handle);
  ASSERT_EQ(ret, 0);
  // get output info
  BPUModelInfo output_info;
  ret = BPU_getModelOutputInfo(bpu_handle, model_name, &output_info);
  LOGI << "Get model output info: " << BPU_getLastError(bpu_handle);
  ASSERT_EQ(ret, 0);
}

TEST(FasterRCNNTest, BasicAPIs) {
  FasterRCNNMethod faster_rcnn_method;
  std::string config_file = "./configs/multitask_config.json";
  auto ret = faster_rcnn_method.Init(config_file);
  EXPECT_EQ(ret, 0);
  auto faster_rcnn_param =
    std::make_shared<xstream::FasterRCNNParam>("FasterRCNNMethod");
  faster_rcnn_param->max_face_count = 1;
  ret = faster_rcnn_method.UpdateParameter(faster_rcnn_param);
  EXPECT_EQ(ret, 0);
  auto param = faster_rcnn_method.GetParameter();
  EXPECT_EQ(param->unique_name_, "FasterRCNNMethod");
  auto version = faster_rcnn_method.GetVersion();
  EXPECT_FALSE(version.empty());
  auto method_info = faster_rcnn_method.GetMethodInfo();
  EXPECT_FALSE(method_info.is_thread_safe_);
  EXPECT_TRUE(method_info.is_need_reorder);
  faster_rcnn_method.OnProfilerChanged(true);
}
