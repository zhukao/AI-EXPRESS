/**
* @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
* @file      disable_method_main.cc
* @brief     example for making method instance disable
* @author    wenhao.zou (wenhao.zou@horizon.ai)
* @date      2020/2/7
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

namespace DisableMethodTutorials {
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
}  // namespace DisableMethodTutorials

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    std::cout << "Usage : ./disable_method_example config" << std::endl;
    std::cout << "Example : ./disable_method_example ./config/control_workflow"
      "_disablemethod.json" << std::endl;
    return -1;
  }
  auto config = argv[1];
  // Create xstream sdk object
  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();

  DisableMethodTutorials::Callback callback;
  std::cout << "config_file :" << config << std::endl;
  flow->SetConfig("config_file", config);
  flow->Init();

  // prepare input data
  InputDataPtr inputdata(new InputData());
  BaseDataVector *data(new BaseDataVector);
  xstream::BBox *bbox1(new xstream::BBox(
    hobot::vision::BBox(0, 0, 40, 40)));
  bbox1->type_ = "BBox";
  data->name_ = "face_head_box";
  data->datas_.push_back(BaseDataPtr(bbox1));
  inputdata->datas_.push_back(BaseDataPtr(data));
  // 1 common predict
  auto out = flow->SyncPredict(inputdata);
  callback.OnCallback(out);

  // 2 set DisableParam Invalid for "BBoxFilter_3"
  std::cout << "------------ invalid output------------" << std::endl;
  // DisableParam set mode with default value Mode::Invalid
  xstream::InputParamPtr invalidFilter3(
    new xstream::DisableParam("BBoxFilter_3"));
  // add invalid parameter to inputdata params_
  inputdata->params_.push_back(invalidFilter3);
  // start synchronous predict, and then get output
  out = flow->SyncPredict(inputdata);
  callback.OnCallback(out);


  // 3 set DisableParam Invalid output "BBoxFilter_2"
  std::cout << "------------ invalid output------------" << std::endl;
  // DisableParam set mode with default value Mode::Invalid
  xstream::InputParamPtr invalidFilter2(
    new xstream::DisableParam("BBoxFilter_2"));
  // add invalid parameter to inputdata params_
  inputdata->params_.clear();
  inputdata->params_.push_back(invalidFilter2);
  // start synchronous predict, and then get output
  out = flow->SyncPredict(inputdata);
  callback.OnCallback(out);
  // destruct xstream sdk object
  delete flow;
  return 0;
}
