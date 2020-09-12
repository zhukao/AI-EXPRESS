/**
 * * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: apa_test.cpp
 * @Brief: unit test
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-09-01 12:27:05
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-09-01 15:18:10
 */

#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <unistd.h>
#include "hobotxsdk/xstream_sdk.h"
#include "hobotxstream/method_factory.h"
#include "PredictMethod/PredictMethod.h"
#include "hobotlog/hobotlog.hpp"
#include "horizon/vision_type/vision_type.hpp"

#ifdef X2
#include "./vio_wrapper.h"
#endif
#ifdef X3
#include "./vio_wrapper_global.h"
#endif

typedef std::shared_ptr<hobot::vision::ImageFrame> ImageFramePtr;

namespace xstream {
namespace method_factory {
MethodPtr CreateMethod(const std::string &method_name) {
  if ("PredictMethod" == method_name) {
    return MethodPtr(new PredictMethod());
  } else {
    return MethodPtr();
  }
}
}   // namespace method_factory
}   // namespace xstream

int RoiInput(std::string cfg_file,
                         std::string fb_cfg,
                         std::string img_list) {
  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();
  int ret = flow->SetConfig("config_file", cfg_file.c_str());
  if (ret != 0) return -1;

  ret = flow->Init();
  if (ret != 0) return -1;
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

TEST(APA_TEST, ParkingLock) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  std::string cfg_file = "./config/parking_lock.json";
  std::string fb_cfg = "./config/vio_config/vio_onsemi0230_fb.json";
  #ifdef X3_X2_VIO
  fb_cfg = "./config/vio_config/hb_vio_x3_1080_fb.json";
  #endif
  #ifdef X3_IOT_VIO
  fb_cfg = "./config/vio_config/vio/x3dev/iot_vio_x3_1080_fb.json";
  #endif
  std::string img_list = "./data/parking_lock/img_rect.txt";

  RoiInput(cfg_file, fb_cfg, img_list);
}
