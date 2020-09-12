/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      main.cc
 * @brief     main function
 * @author    Qingpeng Liu (qingpeng.liu@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-01-09
 */

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include "hobotxstream/xstream_config.h"
#include "hobotxsdk/xstream_error.h"
#include "hobotxsdk/xstream_sdk.h"
#include "xstream/tutorials/stage1/callback.h"
#include "xstream/tutorials/stage1/filter_param.h"
#include "xstream/tutorials/stage1/method/b_box.h"

int main(int argc, char const *argv[]) {
  using xstream::BaseData;
  using xstream::BaseDataPtr;
  using xstream::BaseDataVector;
  using xstream::InputData;
  using xstream::InputDataPtr;
  using Stage1Async::Callback;
  if (argc < 2) {
    std::cout << "Usage : ./bbox_filter_main work_flow_config_file"
              << std::endl;
    std::cout << "Example : ./bbox_filter_main ./filter_workflow.json"
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
  // BBoxFilter_A回调函数
  flow->SetCallback(
      std::bind(&Callback::OnCallback, &callback, std::placeholders::_1),
      "BBoxFilter_A");
  // BBoxFilter_B回调函数
  flow->SetCallback(
      std::bind(&Callback::OnCallback, &callback, std::placeholders::_1),
      "BBoxFilter_B");

  // Get Method Version
  std::cout << "BBoxFilter_A Method Version : "
            << flow->GetVersion("BBoxFilter_A") << std::endl;

  float x1{0};   // BBox(框)的左上角横坐标
  float y1{20};  // BBox(框)的左上角纵坐标
  float x2{0};   // BBox(框)的右上角横坐标
  float y2{50};  // BBox(框)的右上角纵坐标
  // 框的面积计算公式:(x2-x2)*(y2-y1)
  if (argc == 2) {
    std::cout << "***********************" << std::endl
              << "testing synchronous function" << std::endl
              << "***********************" << std::endl;
    // 生成面积为{ 0, 30, 60, 90, 120, 150, 180, 210, 240,
    // 270 } 序列,作为BBoxFilter的输入数据
    for (int i = 0; i < 10; i++) {
      x2 = i;
      InputDataPtr inputdata(new InputData());
      BaseDataVector *data(new BaseDataVector);
      xstream::BBox *bbox(
          new xstream::BBox(hobot::vision::BBox(x1, y1, x2, y2)));
      bbox->type_ = "BBox";
      std::cout << "i:" << i << " bbox:" << bbox->value << std::endl;
      data->datas_.push_back(BaseDataPtr(bbox));

      data->name_ = "in_bbox";
      inputdata->datas_.push_back(BaseDataPtr(data));

      flow->AsyncPredict(inputdata);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));

      if (i == 5) {
        std::string unique_name("BBoxFilter_A");
        auto ptr = std::make_shared<xstream::FilterParam>(unique_name);
        ptr->SetThreshold(400.0);
        flow->UpdateConfig(ptr->unique_name_, ptr);
      }
    }
    auto node_config = flow->GetConfig("BBoxFilter_A");
    if (node_config) {
      auto real_ptr = dynamic_cast<xstream::FilterParam *>(node_config.get());
      std::cout << "threshold:" << real_ptr->GetThreshold() << std::endl;
    }
  }

  delete flow;
  return 0;
}
