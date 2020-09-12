/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      main.cc
 * @brief     main function
 * @author    Qingpeng Liu (qingpeng.liu@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-02-24
 */

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include "xstream/tutorials/stage3/orderdata.h"
#include "hobotxstream/xstream_config.h"
#include "hobotxsdk/xstream_error.h"
#include "hobotxsdk/xstream_sdk.h"
#include "xstream/tutorials/stage3/callback.h"

int main(int argc, char const *argv[]) {
  using xstream::BaseData;
  using xstream::BaseDataPtr;
  using xstream::BaseDataVector;
  using xstream::InputData;
  using xstream::InputDataPtr;
  using Stage3Async::Callback;
  if (argc < 2) {
    std::cout << "Usage : ./thread_safety_reorder_main work_flow_config_file"
              << std::endl;
    std::cout << "Example : ./thread_safety_reorder_main ./workflow.json"
              << std::endl;
    return -1;
  }
  auto config = argv[1];
  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();
  Callback callback;
  // 整个Workflow回调函数
  flow->SetCallback(
      std::bind(&Callback::OnCallback, &callback, std::placeholders::_1));
  flow->SetConfig("config_file", config);
  flow->Init();

  // Get Method Version
  std::cout << "Thread Safety Method version : "
            << flow->GetVersion("thread_safety_example") << std::endl;

  xstream::InputParamPtr
    pass_through(new xstream::DisableParam("passthrough_example",
    xstream::DisableParam::Mode::PassThrough));
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  if (argc == 2) {
    std::cout << "***********************" << std::endl
              << "testing asynchronous function" << std::endl
              << "***********************" << std::endl;
    for (int i = 0; i < 10; i++) {
      InputDataPtr inputdata(new InputData());
      BaseDataVector *data(new BaseDataVector);
      xstream::OrderData *example_data(new xstream::OrderData(i));
      example_data->name_ = "input_data";
      data->name_ = "input_data";
      data->datas_.push_back(BaseDataPtr(example_data));
      inputdata->datas_.push_back(BaseDataPtr(data));
      inputdata->params_.push_back(pass_through);
      flow->AsyncPredict(inputdata);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
  // waiting for async function done
  std::this_thread::sleep_for(std::chrono::seconds(5));

  delete flow;
  return 0;
}
