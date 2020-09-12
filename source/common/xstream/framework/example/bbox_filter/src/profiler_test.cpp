/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file profiler_test.cpp
 * @brief
 * @author ruoting.ding
 * @email ruoting.ding@horizon.ai
 * @date 2019/4/17
 */

#include <chrono>
#include <iostream>
#include <thread>

#include "callback.h"
#include "hobotxsdk/xstream_sdk.h"
#include "hobotxstream/data_types/bbox.h"
#include "hobotxstream/profiler.h"

int main(int argc, char const *argv[]) {
  using xstream::BaseData;
  using xstream::BaseDataPtr;
  using xstream::BaseDataVector;
  using xstream::InputData;
  using xstream::InputDataPtr;

  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();
  xstream::XStreamSDK *flow2 = xstream::XStreamSDK::CreateSDK();

  int target = 10;
  Callback callback(target);
  flow->SetCallback(
      std::bind(&Callback::OnCallback, &callback, std::placeholders::_1));
  flow->SetConfig("config_file", "./config/filter.json");
  flow->SetConfig("profiler", "on");
  flow->SetConfig("profiler_name", "flow");
  flow->SetConfig("profiler_file", "./profiler.txt");
  flow->SetConfig("profiler_frame_interval", "8");
  flow->SetConfig("profiler_time_interval", "100");
  flow->Init();

  flow2->SetCallback(
      std::bind(&Callback::OnCallback, &callback, std::placeholders::_1));
  flow2->SetConfig("config_file", "./config/filter_2.json");
  flow2->SetConfig("profiler", "on");
  flow2->SetConfig("profiler_name", "flow2");
  flow2->SetConfig("profiler_file", "./profiler_2.txt");
  flow2->SetConfig("profiler_frame_interval", "8");
  flow2->SetConfig("profiler_time_interval", "100");
  flow2->Init();
  InputDataPtr inputdata(new InputData());
  BaseDataVector *data(new BaseDataVector);
  xstream::BBox *bbox1(
      new xstream::BBox(hobot::vision::BBox(0, 0, 1000, 1000)));
  bbox1->type_ = "BBox";
  data->name_ = "face_head_box";
  data->datas_.push_back(BaseDataPtr(bbox1));
  inputdata->datas_.push_back(BaseDataPtr(data));

  for (auto i = 0; i < target; ++i) {
    flow->AsyncPredict(inputdata);
    flow2->SyncPredict(inputdata);
  }
  callback.OnReady();
  delete flow;
  delete flow2;
  // 初始化sdk
  return 0;
}
