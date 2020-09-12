/**
* Copyright (c) 2020 Horizon Robotics. All rights reserved.
* @file best_effort_pass_through_method_main.cc
* @brief  example for making method instance pass through with best effort
* @author wenhao.zou
* @email wenhao.zou@horizon.ai
* @date 2020/2/8
*/

#include <unistd.h>
#include <iostream>
#include "hobotxstream/xstream_config.h"
#include "hobotxsdk/xstream_data.h"
#include "hobotxsdk/xstream_sdk.h"
#include "method/bbox.h"

using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::InputData;
using xstream::InputDataPtr;
using xstream::InputParam;
using xstream::InputParamPtr;

namespace BestEffortPassThoughPredefinedMethodTutorials {
class Callback {
 public:
  void OnCallback(xstream::OutputDataPtr output) {
    using xstream::BaseDataVector;
    std::cout << "============Output Call Back============" << std::endl;
    std::cout << "—seq: " << output->sequence_id_ << std::endl;
    std::cout << "—output_type: " << output->output_type_ << std::endl;
    std::cout << "—error_code: " << output->error_code_ << std::endl;
    std::cout << "—error_detail_: " << output->error_detail_ << std::endl;
    std::cout << "—datas_ size: " << output->datas_.size() << std::endl;
    for (auto data : output->datas_) {
      std::cout<< "——output data " << data->name_ << " state:"
      << static_cast<std::underlying_type<xstream::DataState>::type>(
      data->state_) << std::endl;
      if (data->error_code_ < 0) {
        std::cout << "——data error: " << data->error_code_ << std::endl;
        continue;
      }
      std::cout << "——data type:" << data->type_ << " name:" << data->name_
        << std::endl;
      BaseDataVector *pdata = reinterpret_cast<BaseDataVector *>(data.get());
      std::cout << "pdata size: " << pdata->datas_.size() << std::endl;
    }
    std::cout << "============Output Call Back End============" << std::endl;
  }
};
}  // namespace BestEffortPassThoughPredefinedMethodTutorials

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    std::cout << "Usage : ./best_effort_pass_though_method_example config"
      << std::endl;
    std::cout << "Example : ./best_effort_pass_though_method_example "
      "./config/filter.json" << std::endl;
    return -1;
  }

  auto config = argv[1];
  // 1 Create xstream sdk object
  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();

  BestEffortPassThoughPredefinedMethodTutorials::Callback callback;
  flow->SetConfig("config_file", config);
  flow->SetCallback(
    std::bind(
      &BestEffortPassThoughPredefinedMethodTutorials::Callback::OnCallback,
      &callback,
      std::placeholders::_1));
  flow->Init();

  // 2 prepare input data
  InputDataPtr inputdata(new InputData());
  BaseDataVector *data1(new BaseDataVector);
  xstream::BBox *bbox1(new xstream::BBox(
    hobot::vision::BBox(0, 0, 40, 40)));
  bbox1->type_ = "BBox";
  data1->name_ = "face_head_box";
  data1->datas_.push_back(BaseDataPtr(bbox1));
  inputdata->datas_.push_back(BaseDataPtr(data1));

  BaseDataVector *data2(new BaseDataVector);
  xstream::BBox *bbox2(new xstream::BBox(
    hobot::vision::BBox(0, 0, 50, 50)));
  bbox2->type_ = "BBox";
  data2->name_ = "face_head_box2";
  data2->datas_.push_back(BaseDataPtr(bbox2));
  inputdata->datas_.push_back(BaseDataPtr(data2));

  // 3 common
  flow->AsyncPredict(inputdata);

  // 4 set through output
  std::cout << "------------ pass through output----------"
    << std::endl;
  xstream::InputParamPtr
    pass_through(
      new xstream::DisableParam(
        "BBoxFilter_1",
        xstream::DisableParam::Mode::PassThrough));
  inputdata->params_.push_back(pass_through);
  flow->AsyncPredict(inputdata);

  // 5 set best pass through output
  std::cout << "------------best effort pass through output----------"
    << std::endl;
  inputdata->params_.clear();
  xstream::InputParamPtr
    b_effort_pass_through(
      new xstream::DisableParam(
        "BBoxFilter_1",
        xstream::DisableParam::Mode::BestEffortPassThrough));
  inputdata->params_.push_back(b_effort_pass_through);
  flow->AsyncPredict(inputdata);
  // callback.OnCallback(out);
  usleep(1000000);
  // destruct xstream sdk object
  delete flow;
  return 0;
}
