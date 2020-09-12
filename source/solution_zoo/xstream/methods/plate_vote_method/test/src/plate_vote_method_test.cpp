//
// Created by yaoyao.sun on 2019-05-14.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#include "plate_vote_method/plate_vote_method.h"

#include <gtest/gtest.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision_type/vision_type.hpp"

using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::InputData;
using xstream::InputDataPtr;
using xstream::InputParamPtr;
using xstream::XStreamData;
using hobot::vision::BBox;

using xstream::PlateVoteMethod;
TEST(PLATE_VOTE_TEST, Basic) {
  PlateVoteMethod plate_vote_method;

  std::string config_file = "./config/plate_vote.json";
  plate_vote_method.Init(config_file);

  std::vector<std::vector<BaseDataPtr>> xstream_output;
  std::vector<int> number1(7, 1);
  std::vector<int> number2(7, 2);
  for (int i = 0; i < 4; ++i) {
    std::shared_ptr<BaseDataVector> plate_rects_ptr(new BaseDataVector());
    std::shared_ptr<XStreamData<BBox>> box(new XStreamData<BBox>());
    box->value.id = 1;
    plate_rects_ptr->datas_.push_back(box);

    std::shared_ptr<BaseDataVector> plate_ptr(new BaseDataVector);
    std::shared_ptr<XStreamData<std::vector<int>>> plate(
        new XStreamData<std::vector<int>>());
    plate->value = number1;
    plate_ptr->datas_.push_back(plate);

    std::shared_ptr<BaseDataVector> disappeared_track_ids_ptr(
        new BaseDataVector);

    std::vector<std::vector<BaseDataPtr>> input;
    std::vector<xstream::InputParamPtr> param;
    input.resize(1);
    input[0].push_back(plate_rects_ptr);
    input[0].push_back(disappeared_track_ids_ptr);
    input[0].push_back(plate_ptr);

    xstream_output = plate_vote_method.DoProcess(input, param);
  }

  {
    ASSERT_EQ(xstream_output.size(), static_cast<std::size_t>(1));
    auto one_frame_out = xstream_output[0];
    ASSERT_EQ(one_frame_out.size(), static_cast<std::size_t>(1));
    auto out_plate_ptr = one_frame_out[0];
    auto out_plate =
        std::static_pointer_cast<BaseDataVector>(out_plate_ptr)->datas_;
    ASSERT_EQ(out_plate.size(), static_cast<std::size_t>(1));
    auto plate_result =
        std::static_pointer_cast<XStreamData<std::vector<int>>>(out_plate[0])
            ->value;
    auto iter1 = number1.begin();
    auto iter2 = plate_result.begin();
    for (; iter2 != plate_result.end(); iter1++, iter2++) {
      ASSERT_EQ(*iter1, *iter2);
    }
  }

  for (int i = 0; i < 5; ++i) {
    std::shared_ptr<BaseDataVector> plate_rects_ptr(new BaseDataVector());
    std::shared_ptr<XStreamData<BBox>> box(new XStreamData<BBox>());
    box->value.id = 1;
    plate_rects_ptr->datas_.push_back(box);

    std::shared_ptr<BaseDataVector> plate_ptr(new BaseDataVector);
    std::shared_ptr<XStreamData<std::vector<int>>> plate(
        new XStreamData<std::vector<int>>());
    plate->value = number2;
    plate_ptr->datas_.push_back(plate);

    std::shared_ptr<BaseDataVector> disappeared_track_ids_ptr(
        new BaseDataVector);

    std::vector<std::vector<BaseDataPtr>> input;
    std::vector<xstream::InputParamPtr> param;
    input.resize(1);
    input[0].push_back(plate_rects_ptr);
    input[0].push_back(disappeared_track_ids_ptr);
    input[0].push_back(plate_ptr);

    xstream_output = plate_vote_method.DoProcess(input, param);
  }

  {
    ASSERT_EQ(xstream_output.size(), static_cast<std::size_t>(1));
    auto one_frame_out = xstream_output[0];
    ASSERT_EQ(one_frame_out.size(), static_cast<std::size_t>(1));
    auto out_plate_ptr = one_frame_out[0];
    auto out_plate =
        std::static_pointer_cast<BaseDataVector>(out_plate_ptr)->datas_;
    ASSERT_EQ(out_plate.size(), static_cast<std::size_t>(1));
    auto plate_result =
        std::static_pointer_cast<XStreamData<std::vector<int>>>(out_plate[0])
            ->value;
    auto iter1 = number2.begin();
    auto iter2 = plate_result.begin();
    for (; iter2 != plate_result.end(); iter1++, iter2++) {
      ASSERT_EQ(*iter1, *iter2);
    }
  }
  plate_vote_method.Finalize();
}

TEST(PLATE_VOTE_TEST, GetInfo) {
  PlateVoteMethod plate_vote_method;
  std::string config_file = "./config/plate_vote.json";
  auto ret = plate_vote_method.Init(config_file);
  ASSERT_EQ(ret, 0);

  auto param = plate_vote_method.GetParameter();
  ASSERT_TRUE(param == nullptr);
  auto version = plate_vote_method.GetVersion();
  ASSERT_EQ(version, "");
  auto plate_vote_param =
      std::make_shared<xstream::PlateVoteParam>("PlateVoteMethod");
  ret = plate_vote_method.UpdateParameter(plate_vote_param);
  ASSERT_EQ(ret, 0);

  plate_vote_method.Finalize();
}
