/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file disable_method_test.cpp
 * @brief
 * @author ruoting.ding
 * @email ruoting.ding@horizon.ai
 * @date 2019/4/19
 */

#include <chrono>
#include <iostream>
#include <thread>

#include "callback.h"
#include "hobotxsdk/xstream_sdk.h"
#include "hobotxstream/data_types/bbox.h"

using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::InputData;
using xstream::InputDataPtr;
using xstream::InputParam;
using xstream::InputParamPtr;

int main(int argc, char const *argv[]) {
#if 0
  if (argc < 2) {
    printf("Usage : ./disable_method_test config\n");
    printf("Example : ./disable_method_test ./config/filter.json\n");
    return -1;
  }
  auto config = argv[1];
#endif
  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();

  Callback callback(1);
  flow->SetCallback(
      std::bind(&Callback::OnCallback, &callback, std::placeholders::_1));
  // flow->SetConfig("config_file", config);
  flow->SetConfig("config_file", "./config/filter.json");
  flow->Init();

  InputDataPtr inputdata(new InputData());
  BaseDataVector *data(new BaseDataVector);
  xstream::BBox *bbox1(new xstream::BBox(hobot::vision::BBox(0, 0, 10, 10)));
  bbox1->type_ = "BBox";
  data->name_ = "face_head_box";
  data->datas_.push_back(BaseDataPtr(bbox1));
  inputdata->datas_.push_back(BaseDataPtr(data));
  // 1 invalid output
  std::cout << "------------test invalid output------------" << std::endl;
  xstream::InputParamPtr invalid(new xstream::DisableParam("BBoxFilter_1"));
  inputdata->params_.push_back(invalid);
  auto out = flow->SyncPredict(inputdata);
  callback.OnCallback(out);

  // 2 pass through output
  std::cout << "------------test pass through output----------" << std::endl;
  inputdata->params_.clear();
  xstream::InputParamPtr pass_through(new xstream::DisableParam(
      "BBoxFilter_1", xstream::DisableParam::Mode::PassThrough));
  inputdata->params_.push_back(pass_through);
  out = flow->SyncPredict(inputdata);
  callback.OnCallback(out);

  // 3 use pre-defined input data
  std::cout << "------------test pre-defined output----------" << std::endl;
  inputdata->params_.clear();
  xstream::DisableParamPtr pre_define(new xstream::DisableParam(
      "BBoxFilter_1", xstream::DisableParam::Mode::UsePreDefine));
  BaseDataVector *pre_data(new BaseDataVector);
  xstream::BBox *pre_bbox1(
      new xstream::BBox(hobot::vision::BBox(0, 0, 20, 20)));
  pre_bbox1->type_ = "BBox";
  pre_data->name_ = "face_head_box";
  pre_data->datas_.push_back(BaseDataPtr(pre_bbox1));

  pre_define->pre_datas_.emplace_back(BaseDataPtr(pre_data));

  inputdata->params_.push_back(pre_define);
  out = flow->SyncPredict(inputdata);
  callback.OnCallback(out);

  delete flow;
  // 初始化sdk
  return 0;
}
