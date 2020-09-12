/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: gtest_basic.cpp
 * @Brief: definition of the gtest_basic
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-05-25 21:27:05
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-05-25 22:17:08
 */

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_sdk.h"
#include "hobotxsdk/xstream_data.h"
#include "horizon/vision_type/vision_type.hpp"

typedef xstream::XStreamData<hobot::vision::BBox> XStreamBBox;
typedef xstream::XStreamData<hobot::vision::Landmarks> XStreamLandmarks;

using xstream::XStreamData;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::InputData;
using xstream::InputDataPtr;


int getBodyKps(std::string kps_file, std::shared_ptr<XStreamLandmarks> kps) {
  std::ifstream ifs(kps_file);
  if (!ifs.is_open()) {
    LOGD << "open image list file failed." << std::endl;
    return -1;
  }

  std::string gt_data;
  int i = 0;
  while (getline(ifs, gt_data)) {
    if (i >= 17) return -1;
    std::istringstream gt(gt_data);
    gt >> kps->value.values[i].x >> kps->value.values[i].y
       >> kps->value.values[i].score;
    i++;
  }
  return 0;
}

TEST(XStreamSDKTest, RaiseHand) {
  InputDataPtr input(new InputData());

  auto body_box_list = std::make_shared<BaseDataVector>();
  body_box_list->name_ = "body_box_list";
  input->datas_.push_back(BaseDataPtr(body_box_list));

  auto disappeared_track_id = std::make_shared<BaseDataVector>();
  disappeared_track_id->name_ = "disappeared_track_id";
  input->datas_.push_back(BaseDataPtr(disappeared_track_id));

  auto rgb_lmk = std::make_shared<BaseDataVector>();
  rgb_lmk->name_ = "kps";
  input->datas_.push_back(BaseDataPtr(rgb_lmk));

  auto body_box = std::make_shared<XStreamBBox>();
  body_box->type_ = "BBox";
  body_box->value.id = 11;
  body_box->value.x1 = 155;
  body_box->value.y1 = 9;
  body_box->value.x2 = 266;
  body_box->value.y2 = 513;

  body_box_list->datas_.emplace_back(BaseDataPtr(body_box));
  body_box_list->datas_.emplace_back(BaseDataPtr(body_box));

  auto body_landmarks = std::make_shared<XStreamLandmarks>();
  body_landmarks->type_ = "Landmarks";
  body_landmarks->value.values.resize(17);
  getBodyKps("./test/data/raisehand.txt", body_landmarks);
  rgb_lmk->datas_.emplace_back(BaseDataPtr(body_landmarks));

  auto body_landmarks1 = std::make_shared<XStreamLandmarks>();
  body_landmarks1->type_ = "Landmarks";
  body_landmarks1->value.values.resize(17);
  // not raise hand, from test/data/stand.jpg
  getBodyKps("./test/data/stand.txt", body_landmarks1);
  rgb_lmk->datas_.emplace_back(BaseDataPtr(body_landmarks1));

  auto flow = xstream::XStreamSDK::CreateSDK();
  flow->SetConfig("config_file", "./config/raisehand_method.json");
  flow->Init();
  auto out = flow->SyncPredict(input);

  // std::vector<BaseDataPtr> out->datas_
  EXPECT_EQ(out->error_code_, 0);
  EXPECT_EQ(out->datas_.size(), static_cast<size_t>(1));


  auto raise_hands = dynamic_cast<BaseDataVector *>(out->datas_[0].get());
  ASSERT_TRUE(!raise_hands->datas_.empty());
  std::cout << "raise_hands->datas_.size(): "
            << raise_hands->datas_.size() << std::endl;
  EXPECT_EQ(raise_hands->datas_.size(), static_cast<size_t>(2));

  for (size_t i = 0; i < raise_hands->datas_.size(); ++i) {
    auto attribute = std::static_pointer_cast<XStreamData<
                       hobot::vision::Attribute<int32_t>>>(
                         raise_hands->datas_[i]);
    std::cout << "Raisehand: " << attribute->value.value << std::endl;
    if (i == 0) {
      EXPECT_EQ(attribute->value.value, static_cast<int32_t>(1));
    } else {
      EXPECT_EQ(attribute->value.value, static_cast<int32_t>(0));
    }
  }
}

TEST(XStreamSDKTest, StandSquat) {
  InputDataPtr input(new InputData());

  auto body_box_list = std::make_shared<BaseDataVector>();
  body_box_list->name_ = "body_box_list";
  input->datas_.push_back(BaseDataPtr(body_box_list));

  auto disappeared_track_id = std::make_shared<BaseDataVector>();
  disappeared_track_id->name_ = "disappeared_track_id";
  input->datas_.push_back(BaseDataPtr(disappeared_track_id));

  auto rgb_lmk = std::make_shared<BaseDataVector>();
  rgb_lmk->name_ = "kps";
  input->datas_.push_back(BaseDataPtr(rgb_lmk));

  auto body_box = std::make_shared<XStreamBBox>();
  body_box->type_ = "BBox";
  body_box->value.id = 11;
  body_box->value.x1 = 155;
  body_box->value.y1 = 9;
  body_box->value.x2 = 266;
  body_box->value.y2 = 513;

  body_box_list->datas_.emplace_back(BaseDataPtr(body_box));
  body_box_list->datas_.emplace_back(BaseDataPtr(body_box));

  auto body_landmarks = std::make_shared<XStreamLandmarks>();
  body_landmarks->type_ = "Landmarks";
  body_landmarks->value.values.resize(17);
  // squat, from test/data/squat.jpg
  getBodyKps("./test/data/squat.txt", body_landmarks);
  rgb_lmk->datas_.emplace_back(BaseDataPtr(body_landmarks));

  auto body_landmarks1 = std::make_shared<XStreamLandmarks>();
  body_landmarks1->type_ = "Landmarks";
  body_landmarks1->value.values.resize(17);
  // stand, from test/data/stand.jpg
  getBodyKps("./test/data/stand.txt", body_landmarks1);
  rgb_lmk->datas_.emplace_back(BaseDataPtr(body_landmarks1));

  // Stand
  {
    auto flow1 = xstream::XStreamSDK::CreateSDK();
    flow1->SetConfig("config_file", "./config/stand_method.json");
    flow1->Init();
    auto out = flow1->SyncPredict(input);

    (out->error_code_, 0);
    EXPECT_EQ(out->datas_.size(), static_cast<size_t>(1));

    auto stands = dynamic_cast<BaseDataVector *>(out->datas_[0].get());
    ASSERT_TRUE(!stands->datas_.empty());
    std::cout << "stands->datas_.size(): "
              << stands->datas_.size() << std::endl;
    EXPECT_EQ(stands->datas_.size(), static_cast<size_t>(2));

    for (size_t i = 0; i < stands->datas_.size(); ++i) {
      auto attribute = std::static_pointer_cast<XStreamData<
                         hobot::vision::Attribute<int32_t>>>(
                           stands->datas_[i]);
      std::cout << "Stand: " << attribute->value.value << std::endl;
      if (i == 0) {
        EXPECT_EQ(attribute->value.value, static_cast<int32_t>(0));
      } else {
        EXPECT_EQ(attribute->value.value, static_cast<int32_t>(1));
      }
    }
  }

  // Squat
  {
    auto flow2 = xstream::XStreamSDK::CreateSDK();
    flow2->SetConfig("config_file", "./config/squat_method.json");
    flow2->Init();
    auto out = flow2->SyncPredict(input);

    EXPECT_EQ(out->error_code_, 0);
    EXPECT_EQ(out->datas_.size(), static_cast<size_t>(1));

    auto squats = dynamic_cast<BaseDataVector *>(out->datas_[0].get());
    ASSERT_TRUE(!squats->datas_.empty());
    std::cout << "squats->datas_.size(): "
              << squats->datas_.size() << std::endl;
    EXPECT_EQ(squats->datas_.size(), static_cast<size_t>(2));

    for (size_t i = 0; i < squats->datas_.size(); ++i) {
      auto attribute = std::static_pointer_cast<XStreamData<
                         hobot::vision::Attribute<int32_t>>>(
                           squats->datas_[i]);
      std::cout << "Stand: " << attribute->value.value << std::endl;
      if (i == 0) {
        EXPECT_EQ(attribute->value.value, static_cast<int32_t>(1));
      } else {
        EXPECT_EQ(attribute->value.value, static_cast<int32_t>(0));
      }
    }
  }
}
