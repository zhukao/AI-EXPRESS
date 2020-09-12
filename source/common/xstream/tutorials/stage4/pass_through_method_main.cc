/**
* Copyright (c) 2020 Horizon Robotics. All rights reserved.
* @file pass_through_method_main.cc
* @brief  example for making method instance pass through
* @author wenhao.zou
* @email wenhao.zou@horizon.ai
* @date 2020/2/7
*/

#include <unistd.h>
#include <iostream>
#include "hobotxstream/xstream_config.h"
#include "hobotxsdk/xstream_data.h"
#include "hobotxsdk/xstream_sdk.h"
#include "method/bbox.h"

using xstream::InputParam;
using xstream::InputParamPtr;
using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::InputData;
using xstream::InputDataPtr;

namespace PassThroughMethodTutorials {
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
      // BaseDataVector *pdata = reinterpret_cast<BaseDataVector *>(data.get());
      // std::cout << "pdata size: " << pdata->datas_.size() << std::endl;
    }
    std::cout << "============Output Call Back End============" << std::endl;
  }
};
}  // namespace PassThroughMethodTutorials

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    std::cout << "Usage : ./pass_through_method_example config"
      << std::endl;
    std::cout << "Example : ./pass_through_method_example "
      "./config/control_workflow_pass_through.json" << std::endl;
    return -1;
  }

  auto config = argv[1];
  // 1 Create xstream sdk object
  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();

  PassThroughMethodTutorials::Callback callback;
  flow->SetConfig("config_file", config);
  flow->SetCallback(
    std::bind(&PassThroughMethodTutorials::Callback::OnCallback,
      &callback,
      std::placeholders::_1));
  flow->Init();

  // 2 prepare input data
  InputDataPtr inputdata(new InputData());
  BaseDataVector *data(new BaseDataVector);
  xstream::BBox *bbox1(new xstream::BBox(
    hobot::vision::BBox(0, 0, 60, 60)));
  bbox1->type_ = "BBox";
  data->name_ = "face_head_box";
  data->datas_.push_back(BaseDataPtr(bbox1));
  inputdata->datas_.push_back(BaseDataPtr(data));

  // 3 common
  flow->AsyncPredict(inputdata);
  // callback.OnCallback(out);

  // 4 set pass through output
  std::cout << "------------ pass through output----------" << std::endl;
  inputdata->params_.clear();
  xstream::InputParamPtr
      pass_through(
        new xstream::DisableParam(
          "PostBoxFilter_2",
          xstream::DisableParam::Mode::PassThrough));
  inputdata->params_.push_back(pass_through);
  flow->AsyncPredict(inputdata);
  // callback.OnCallback(out);
  usleep(1000000);
  // destruct xstream sdk object
  delete flow;
  return 0;
}

