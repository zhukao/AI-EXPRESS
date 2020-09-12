/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     Method interface of xstream framework
 * @author    chao.yang
 * @email     chao01.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.25
 */

#include <unistd.h>

#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision_type/vision_type.hpp"

typedef xstream::XStreamData<hobot::vision::BBox> XStreamBBox;
typedef xstream::XStreamData<hobot::vision::Pose3D> XStreamPose3D;
typedef xstream::XStreamData<hobot::vision::Landmarks> XStreamLandmarks;
typedef xstream::XStreamData<hobot::vision::Quality> XStreamQuality;
typedef xstream::XStreamData<float> XStreamFloat;

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
    for (const auto &data : output->datas_) {
      if (data->error_code_ < 0) {
        std::cout << "data error: " << data->error_code_ << std::endl;
        continue;
      }
      std::cout << "data type_name : " << data->type_ << " " << data->name_
                << std::endl;
      auto *pdata = dynamic_cast<BaseDataVector *>(data.get());
      std::cout << "pdata size: " << pdata->datas_.size() << std::endl;
      for (const auto &item : pdata->datas_) {
        assert("Number" == item->type_);
        //  get the item score
        auto select_score = std::static_pointer_cast<XStreamFloat>(item);
        std::cout << "select_score:" << select_score->value << std::endl;
      }
    }
  }
};

int main(int argc, char const *argv[]) {
  using xstream::BaseData;
  using xstream::BaseDataPtr;
  using xstream::BaseDataVector;
  using xstream::InputData;
  using xstream::InputDataPtr;

  // init the SDK
  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();
  Callback callback;
  flow->SetCallback(
      std::bind(&Callback::OnCallback, &callback, std::placeholders::_1));
  flow->SetConfig("config_file", "./config/grading.json");
  flow->Init();

  InputDataPtr inputdata(new InputData());

  // construct the box_list and put it into datas_[0]
  std::shared_ptr<BaseDataVector> box_list(new BaseDataVector());
  box_list->name_ = "box_list";

  std::shared_ptr<XStreamBBox> bbox1(new XStreamBBox());
  bbox1->type_ = "BBox";
  bbox1->value.x1 = 861;
  bbox1->value.y1 = 995;
  bbox1->value.x2 = 917;
  bbox1->value.y2 = 1063;
  bbox1->value.score = 1.084960;
  std::shared_ptr<XStreamBBox> bbox2(new XStreamBBox());
  bbox2->type_ = "BBox";
  bbox2->value.x1 = 1000;
  bbox2->value.y1 = 140;
  bbox2->value.x2 = 1061;
  bbox2->value.y2 = 202;
  bbox2->value.score = 0.9901622;
  box_list->datas_.push_back(bbox1);
  box_list->datas_.push_back(bbox2);
  //  box_list->state_ = xstream::DataState::INVALID;
  inputdata->datas_.push_back(box_list);

  // construct the pose_3d_list and put it into datas_[1]
  std::shared_ptr<BaseDataVector> pose_3d_list(new BaseDataVector());
  pose_3d_list->name_ = "pose_3d_list";

  std::shared_ptr<XStreamPose3D> pose1(new XStreamPose3D());
  pose1->value.pitch = -14.5738907;
  pose1->value.yaw = 19.8864212;
  pose1->value.roll = -6.61847687;

  std::shared_ptr<XStreamPose3D> pose2(new XStreamPose3D());
  pose2->value.pitch = -15.0597382;
  pose2->value.yaw = 2.414505;
  pose2->value.roll = -1.443367;
  pose_3d_list->datas_.push_back(pose1);
  pose_3d_list->datas_.push_back(pose2);
  //  pose_3d_list->state_ = xstream::DataState::INVALID;
  inputdata->datas_.push_back(pose_3d_list);

  // construct the land_mark_list and put it into datas_[2]
  std::shared_ptr<BaseDataVector> land_mark_list(new BaseDataVector());
  land_mark_list->name_ = "land_mark_list";

  std::shared_ptr<XStreamLandmarks> lmk1(new XStreamLandmarks());
  hobot::vision::Point point0(881, 1021, 13);
  hobot::vision::Point point1(907, 1079, 12);
  hobot::vision::Point point2(897, 1036, 13);
  hobot::vision::Point point3(884, 1047, 12);
  hobot::vision::Point point4(905, 1045, 13);
  lmk1->value.values = {point0, point1, point2, point3, point4};

  std::shared_ptr<XStreamLandmarks> lmk2(new XStreamLandmarks());
  hobot::vision::Point point5(1020, 156, 9);
  hobot::vision::Point point6(1047, 159, 4);
  hobot::vision::Point point7(1034, 169, 13);
  hobot::vision::Point point8(1020, 180, 9);
  hobot::vision::Point point9(1041, 182, 9);
  lmk2->value.values = {point5, point6, point7, point8, point9};

  land_mark_list->datas_.push_back(lmk1);
  land_mark_list->datas_.push_back(lmk2);
  //  land_mark_list->state_ = xstream::DataState::INVALID;
  inputdata->datas_.push_back(land_mark_list);

  // construct the quality_list and put it into datas_[3]
  std::shared_ptr<BaseDataVector> quality_list(new BaseDataVector());
  quality_list->name_ = "quality_list";
  std::shared_ptr<XStreamQuality> quality1(new XStreamQuality());
  quality1->value.value = 0;
  //  quality1->value.value = 75;
  //  quality1->value.value = 0.3125f;

  std::shared_ptr<XStreamQuality> quality2(new XStreamQuality());
  quality2->value.value = 0;
  //  quality2->value.value = 11;
  //  quality2->value.value = 0.472499996f;

  quality_list->datas_.push_back(quality1);
  quality_list->datas_.push_back(quality2);
  //  quality_list->state_ = xstream::DataState::INVALID;
  inputdata->datas_.push_back(quality_list);

  // sync GradingMethod
  auto out = flow->SyncPredict(inputdata);
  callback.OnCallback(out);

  // async GradingMethod
  flow->AsyncPredict(inputdata);
  sleep(1);

  delete flow;
  return 0;
}
