/**
* Copyright (c) 2020 Horizon Robotics. All rights reserved.
* @file use_pre_defined_method_main.cc
* @brief  example for making method instance disable
* @author wenhao.zou
* @email wenhao.zou@horizon.ai
* @date 2020/2/7
*/

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

namespace UsePredefinedMethodTutorials {
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
}  // namespace UsePredefinedMethodTutorials

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    std::cout << "Usage : ./use_predefined_method_example config" << std::endl;
    std::cout << "Example : ./use_predefined_method_example "
      "./config/control_workflow_use_predefined.json" << std::endl;
    return -1;
  }

  auto config = argv[1];
  // 1 Create xstream sdk object
  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();

  UsePredefinedMethodTutorials::Callback callback;
  flow->SetCallback(
    std::bind(
      &UsePredefinedMethodTutorials::Callback::OnCallback,
      &callback,
      std::placeholders::_1));
  flow->SetCallback(
    std::bind(
      &UsePredefinedMethodTutorials::Callback::OnCallback,
      &callback,
      std::placeholders::_1), "BBoxFilter_2");

  flow->SetConfig("config_file", config);
  flow->Init();

  // 2 prepare input data
  InputDataPtr inputdata(new InputData());
  BaseDataVector *data(new BaseDataVector);
  xstream::BBox *bbox1(new xstream::BBox(
    hobot::vision::BBox(0, 0, 40, 40)));
  bbox1->type_ = "BBox";
  data->name_ = "face_head_box";
  data->datas_.push_back(BaseDataPtr(bbox1));
  inputdata->datas_.push_back(BaseDataPtr(data));

  // 3 use pre-defined input data
  std::cout << "------------ pre-defined output----------" << std::endl;
  inputdata->params_.clear();
  xstream::DisableParamPtr
      pre_define(
        new xstream::DisableParam(
          "BBoxFilter_1",
          xstream::DisableParam::Mode::UsePreDefine));

  // 4 preprea new input data
  BaseDataVector *pre_data(new BaseDataVector);
  xstream::BBox *pre_bbox1(new xstream::BBox(
    hobot::vision::BBox(0, 0, 20, 20)));
  pre_bbox1->type_ = "BBox";
  pre_data->name_ = "face_head_box";
  pre_data->datas_.push_back(BaseDataPtr(pre_bbox1));
  pre_define->pre_datas_.emplace_back(BaseDataPtr(pre_data));

  inputdata->params_.push_back(pre_define);
  auto out = flow->SyncPredict(inputdata);
  callback.OnCallback(out);

  // destruct xstream sdk object
  delete flow;
  return 0;
}
