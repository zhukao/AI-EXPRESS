/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      main.cc
 * @brief     main function
 * @author    Qingpeng Liu (qingpeng.liu@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-01-03
 */

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "hobotxsdk/xstream_error.h"
#include "hobotxsdk/xstream_sdk.h"
#include "hobotxstream/xstream_config.h"
#include "xstream/tutorials/stage2/filter_param.h"
#include "xstream/tutorials/stage2/method/b_box.h"

void ParseOutput(xstream::OutputDataPtr output) {
  using xstream::BaseDataVector;
  std::cout << "======================" << std::endl;
  std::cout << "seq: " << output->sequence_id_ << std::endl;
  std::cout << "output_type: " << output->output_type_ << std::endl;
  std::cout << "method_unique_name: " << output->unique_name_ << std::endl;
  std::cout << "error_code: " << output->error_code_ << std::endl;
  std::cout << "error_detail_: " << output->error_detail_ << std::endl;
  std::cout << "datas_ size: " << output->datas_.size() << std::endl;

  for (auto data : output->datas_) {
    if (data->error_code_ < 0) {
      std::cout << "data error: " << data->error_code_ << std::endl;
      continue;
    }
    std::cout << "data type_name : " << data->type_ << " " << data->name_
              << std::endl;
    BaseDataVector *pdata = reinterpret_cast<BaseDataVector *>(data.get());
    std::cout << "pdata size: " << pdata->datas_.size() << std::endl;
    std::cout << "Output BBox " << pdata->name_ << ":" << std::endl;
    for (size_t i = 0; i < pdata->datas_.size(); ++i) {
      auto xstream_box =
          std::static_pointer_cast<xstream::XStreamData<hobot::vision::BBox>>(
              pdata->datas_[i]);
      if (xstream_box->state_ == xstream::DataState::VALID) {
        std::cout << "[" << xstream_box->value.x1 << ","
                  << xstream_box->value.y1 << "," << xstream_box->value.x2
                  << "," << xstream_box->value.y2 << "]" << std::endl;
      } else {
        std::cout << "pdata->datas_[ " << i
                  << " ]: state_:" << static_cast<int>(xstream_box->state_)
                  << std::endl;
      }
    }
  }
}

int main(int argc, char const *argv[]) {
  using xstream::BaseData;
  using xstream::BaseDataPtr;
  using xstream::BaseDataVector;
  using xstream::InputData;
  using xstream::InputDataPtr;
  if (argc < 2) {
    std::cout << "Usage : ./bbox_filter_main work_flow_config_file"
              << std::endl;
    std::cout << "Example : ./bbox_filter_main ./filter_workflow.json"
              << std::endl;
    return -1;
  }
  auto config = argv[1];
  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();
  flow->SetConfig("config_file", config);
  flow->Init();
  /// Get Method Version
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

      auto out = flow->SyncPredict(inputdata);
      ParseOutput(out);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  delete flow;
  return 0;
}
