//
// Created by shiyu.fu on 2020-04-08.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#include <gtest/gtest.h>

#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <memory>

#include "FilterSkipFrameMethod/FilterSkipFrameMethod.h"
#include "hobotxsdk/xstream_sdk.h"
#include "hobotxsdk/xstream_data.h"
#include "hobotlog/hobotlog.hpp"
#include "horizon/vision_type/vision_type.hpp"
#include "json/json.h"

using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::InputData;
using xstream::InputDataPtr;
using xstream::XStreamData;
using xstream::InputParamPtr;
using hobot::vision::BBox;


TEST(FilterSkipFrameMethodTest, Basic) {
  xstream::FilterSkipFrameMethod filter_skip_method;
  std::string config_file = "./config/filter_skip_frame.json";
  auto ret = filter_skip_method.Init(config_file);
  EXPECT_EQ(ret, 0);
  auto param = filter_skip_method.GetParameter();
  EXPECT_EQ(param->unique_name_, "filter_skip");

  auto filter_skip_param =
      std::make_shared<xstream::FilterSkipFrameParam>("filter_skip");
  // empty param
  ret = filter_skip_method.UpdateParameter(filter_skip_param);
  EXPECT_EQ(ret, -1);
  // non-empty param
  std::string update_cfg_file = "./config/update_param_test.json";
  std::ifstream ifs(update_cfg_file);
  ASSERT_TRUE(ifs.is_open());
  std::string cfg_str((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());
  filter_skip_param->config_str = cfg_str;
  ret = filter_skip_method.UpdateParameter(filter_skip_param);
  EXPECT_EQ(ret, 0);

  auto version = filter_skip_method.GetVersion();
  EXPECT_EQ(version, "0.0.1");
  filter_skip_method.Finalize();
}

TEST(FilterSkipFrameMethodTest, SkipFrame) {
  xstream::FilterSkipFrameMethod filter_skip_method;
  std::string config_file = "./config/filter_skip_frame.json";
  auto ret = filter_skip_method.Init(config_file);
  EXPECT_EQ(ret, 0);

  std::vector<std::vector<BaseDataPtr>> xstream_output;
  std::shared_ptr<BaseDataVector> rects_ptr(new BaseDataVector());
  std::shared_ptr<XStreamData<BBox>> box(new XStreamData<BBox>());
  box->value.id = 1;
  box->value.x1 = 100;
  box->value.y1 = 100;
  box->value.x2 = 900;
  box->value.y2 = 900;
  box->value.score = 0.9;
  rects_ptr->datas_.push_back(box);

  for (int i = 0; i < 4; ++i) {
    std::vector<std::vector<BaseDataPtr>> input;
    std::vector<xstream::InputParamPtr> param;
    input.resize(1);
    input[0].push_back(rects_ptr);

    xstream_output = filter_skip_method.DoProcess(input, param);
    ASSERT_EQ(xstream_output.size(), static_cast<std::size_t>(1));
    auto one_frame_out = xstream_output[0];
    ASSERT_EQ(one_frame_out.size(), static_cast<std::size_t>(1));
    auto out_rects = std::static_pointer_cast<BaseDataVector>(one_frame_out[0]);
    auto out_rect = out_rects->datas_[0];
    // get skip_num
    auto input_param = filter_skip_method.GetParameter();
    auto method_param =
        dynamic_cast<xstream::FilterSkipFrameParam *>(input_param.get());
    std::stringstream cfg_stream(method_param->config_str);
    Json::Value root;
    Json::CharReaderBuilder reader_builder;
    std::string err_str;
    bool parse_ok =
        Json::parseFromStream(reader_builder, cfg_stream, &root, &err_str);
    EXPECT_TRUE(parse_ok);
    int skip_num = root["skip_num"].asInt();
    if (i == 0) {
      ASSERT_EQ(out_rect->state_, xstream::DataState::VALID);
    } else if (i % skip_num == 0) {
      ASSERT_EQ(out_rect->state_, xstream::DataState::VALID);
    } else {
      ASSERT_EQ(out_rect->state_, xstream::DataState::FILTERED);
    }
  }

  {
    // box outside the border
    std::shared_ptr<BaseDataVector> rects_ptr(new BaseDataVector());
    std::shared_ptr<XStreamData<BBox>> box(new XStreamData<BBox>());
    box->value.id = 2;
    box->value.x1 = 10;
    box->value.y1 = 10;
    box->value.x2 = 50;
    box->value.y2 = 50;
    rects_ptr->datas_.push_back(box);
    std::vector<std::vector<BaseDataPtr>> input;
    std::vector<xstream::InputParamPtr> param;
    input.resize(1);
    input[0].push_back(rects_ptr);

    xstream_output = filter_skip_method.DoProcess(input, param);
    ASSERT_EQ(xstream_output.size(), static_cast<std::size_t>(1));
    auto one_frame_out = xstream_output[0];
    ASSERT_EQ(one_frame_out.size(), static_cast<std::size_t>(1));
    auto out_rects = std::static_pointer_cast<BaseDataVector>(one_frame_out[0]);
    auto out_rect = out_rects->datas_[0];
    ASSERT_EQ(out_rect->state_, xstream::DataState::FILTERED);
  }

  filter_skip_method.Finalize();
}
