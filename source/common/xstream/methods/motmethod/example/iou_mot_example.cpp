/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     Method interface of xsoul framework
 * @author    chao.yang
 * @email     chao01.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.15
 */

#include <unistd.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <cassert>
#include <string>

#include "hobotxsdk/xstream_sdk.h"
#include "hobotxsdk/xstream_data.h"
#include "horizon/vision_type/vision_type.hpp"

typedef xstream::XStreamData<hobot::vision::BBox> XStreamBBox;

typedef xstream::XStreamData<uint32_t> XStreamUint32;

class Callback {
 public:
  Callback() = default;

  ~Callback() = default;

  void OnCallback(const xstream::OutputDataPtr &output) {
    using xstream::BaseDataVector;
    std::cout << "======================" << std::endl;
    std::cout << "seq: " << output->sequence_id_ << std::endl;
    std::cout << "error_code: " << output->error_code_ << std::endl;
    std::cout << "error_detail_: " << output->error_detail_ << std::endl;
    std::cout << "datas_ size: " << output->datas_.size() << std::endl;

    assert(output->datas_.size() >= 2);
    auto &bbox_list = output->datas_[0];
//    auto &disappeared_track_id_list = output->datas_[1];
    if (bbox_list->error_code_ < 0) {
      std::cout << "bbox_list data error: "
      << bbox_list->error_code_ << std::endl;
    }
    std::cout << "bbox_list data type_name : " << bbox_list->type_
              << " " << bbox_list->name_ << std::endl;
    auto bbox_data = std::static_pointer_cast<BaseDataVector>(bbox_list);
    std::cout << "bbox_list data data size: "
              << bbox_data->datas_.size() << std::endl;

    for (auto & pbbox : bbox_data->datas_) {
      assert("BBox" == pbbox->type_);
      auto bbox = std::static_pointer_cast<XStreamBBox>(pbbox);
      std::cout << "id and bbox:"
                << " track_id:" << bbox->value.id
                << " bbox:" <<  bbox->value.x1
                << "," <<  bbox->value.y1
                << "," <<  bbox->value.x2
                << "," <<  bbox->value.y2
                << "," <<  bbox->value.score
                << std::endl;
    }
  }
};

class MotMethodParam : public xstream::InputParam {
 public:
  MotMethodParam(const std::string &method_name,
      const std::string &json_config) : InputParam(method_name) {
    content_ = json_config;
    is_json_format_ = true;
  }
  std::string Format() override { return content_; };
 private:
  std::string content_ = "";
};


int main(int argc, char const *argv[]) {
  using xstream::BaseData;
  using xstream::BaseDataPtr;
  using xstream::BaseDataVector;
  using xstream::InputData;
  using xstream::InputDataPtr;

  // 初始化sdk
  xstream::XStreamSDK* flow = xstream::XStreamSDK::CreateSDK();
  Callback callback;
  flow->SetCallback(
      std::bind(&Callback::OnCallback, &callback, std::placeholders::_1));
  flow->SetConfig("config_file", "../../config/iou_mot.json");
  flow->Init();

  InputDataPtr inputdata(new InputData());

  /* pass-through mode

  xstream::InputParamPtr input_param(
      new MotMethodParam("IOU_example", "pass-through"));
  inputdata->params_.push_back(input_param);

  */

  std::shared_ptr<BaseDataVector> face_head_box(new BaseDataVector);
  std::shared_ptr<XStreamBBox> bbox1(new XStreamBBox());
  bbox1->type_ = "BBox";
  bbox1->value.x1 = 0;
  bbox1->value.y1 = 0;
  bbox1->value.x2 = 1000;
  bbox1->value.y2 = 1000;
  bbox1->value.score = 0.3;

  std::shared_ptr<XStreamBBox> bbox2(new XStreamBBox());
  bbox2->type_ = "BBox";
  bbox2->value.x1 = 0;
  bbox2->value.y1 = 0;
  bbox2->value.x2 = 10;
  bbox2->value.y2 = 10;
  bbox2->value.score = 0.5;
  face_head_box->datas_.push_back(BaseDataPtr(bbox1));
  face_head_box->datas_.push_back(BaseDataPtr(bbox2));

  face_head_box->name_ = "face_head_box_list";
  inputdata->datas_.push_back(BaseDataPtr(face_head_box));

  // async MOTMethod
  flow->AsyncPredict(inputdata);
  sleep(1);

  // sync MOTMethod
  auto out = flow->SyncPredict(inputdata);
  callback.OnCallback(out);

  delete flow;
  return 0;
}
