/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: example_do_det.cpp
 * @Brief:
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-05-22 14:27:05
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-05-22 15:18:10
 */

#include <stdint.h>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include "CNNMethod/CNNMethod.h"
#include "CNNMethod/util/util.h"
#include "FasterRCNNMethod/FasterRCNNMethod.h"
#include "bpu_predict/bpu_io.h"
#include "bpu_predict/bpu_predict.h"
#include "hobotxstream/method.h"
#include "hobotxstream/method_factory.h"
#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision_type/vision_type.hpp"
#include "horizon/vision_type/vision_type_common.h"
#include "json/json.h"
#ifdef X2
#include "./vio_wrapper.h"
#endif
#ifdef X3
#include "./vio_wrapper_global.h"
#endif

typedef std::shared_ptr<hobot::vision::ImageFrame> ImageFramePtr;
struct PyramidResult {
  img_info_t result_info;
};

static void Usage() {
  std::cout << "./example do_det xstream_cfg_file fb_cfg img_list" << std::endl;
}

void PrintFace(std::vector<xstream::BaseDataPtr> &result) {
  auto rois = std::static_pointer_cast<xstream::BaseDataVector>(result[0]);
  for (size_t roi_idx = 0; roi_idx < rois->datas_.size(); roi_idx++) {
    auto roi =
        std::static_pointer_cast<xstream::XStreamData<hobot::vision::BBox>>(
            rois->datas_[roi_idx]);
    std::cout << "x1:" << roi->value.x1 << " y1:" << roi->value.y1
              << " x2:" << roi->value.x2 << " y2:" << roi->value.y2
              << std::endl;
  }
}

int DoDet(int argc, char **argv) {
  if (argc < 4) {
    Usage();
    return 1;
  }
  std::string cfg_file(argv[1]);
  std::string fb_cfg(argv[2]);
  std::string img_list(argv[3]);

  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();
  flow->SetConfig("config_file", cfg_file.c_str());
  flow->SetConfig("profiler", "on");
  flow->SetConfig("profiler_file", "./profiler.txt");
  flow->Init();

#ifdef X2
  img_info_t feed_back_info;
  HbVioFbWrapper fb_handle(fb_cfg);
  fb_handle.Init();

  std::ifstream img_list_file(img_list);
  std::string img_path;
  while (getline(img_list_file, img_path)) {
    uint32_t effective_w, effective_h;
    fb_handle.GetImgInfo(img_path, &feed_back_info, &effective_w, &effective_h);

    xstream::InputDataPtr inputdata(new xstream::InputData());

    auto py_img = std::make_shared<hobot::vision::PymImageFrame>();
    py_img->img = feed_back_info;
    auto xstream_data = std::make_shared<xstream::XStreamData<ImageFramePtr>>();
    xstream_data->value =
        std::static_pointer_cast<hobot::vision::ImageFrame>(py_img);
    xstream_data->name_ = "pyramid";
    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(xstream_data));

    auto out = flow->SyncPredict(inputdata);
    PrintFace(out->datas_);
    fb_handle.FreeImgInfo(&feed_back_info);
  }
#endif
#ifdef X3
  HbVioFbWrapperGlobal fb_handle(fb_cfg);
  fb_handle.Init();

  std::ifstream img_list_file(img_list);
  std::string img_path;
  while (getline(img_list_file, img_path)) {
    uint32_t effective_w, effective_h;
    auto py_img = fb_handle.GetImgInfo(img_path, &effective_w, &effective_h);

    xstream::InputDataPtr inputdata(new xstream::InputData());
    auto xstream_data = std::make_shared<xstream::XStreamData<ImageFramePtr>>();
    xstream_data->value =
        std::static_pointer_cast<hobot::vision::ImageFrame>(py_img);
    xstream_data->name_ = "pyramid";
    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(xstream_data));
    auto out = flow->SyncPredict(inputdata);
    PrintFace(out->datas_);
    fb_handle.FreeImgInfo(py_img);
  }
#endif
  delete flow;
  return 0;
}
