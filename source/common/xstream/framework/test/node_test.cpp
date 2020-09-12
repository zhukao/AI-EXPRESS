/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file disable_method_test.cpp
 * @brief
 * @author shiqing.xie
 * @email shiqing.xie@horizon.ai
 * @date 2019/11/28
 */

#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "hobotxstream/data_types/bbox.h"
#include "hobotxstream/data_types/filter_param.h"
#include "hobotxstream/xstream_config.h"
#include "hobotxsdk/xstream_error.h"
#include "hobotxsdk/xstream_sdk.h"
#include "method/bbox_filter.h"

using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::InputData;
using xstream::InputDataPtr;
using xstream::InputParam;
using xstream::InputParamPtr;

clock_t begin_clock = 0;
namespace NodeTest {
class Callback {
 public:
  Callback() {}

  ~Callback() {}

  void OnCallback(xstream::OutputDataPtr output) {
    using xstream::BaseDataVector;
    if ((output->sequence_id_ == 99999) || (output->sequence_id_ % 1000 == 0)) {
      auto duration = clock() - begin_clock;
      std::cout << "seq: " << output->sequence_id_ << " duration:" << duration
                << std::endl;
    }
    std::cout << "======================" << std::endl;
    std::cout << "seq: " << output->sequence_id_ << std::endl;
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
    }
  }
};
}  // namespace NodeTest

TEST(Interface, PassThrough) {
  auto xstream = xstream::XStreamSDK::CreateSDK();
  ASSERT_TRUE(xstream);
  NodeTest::Callback callback;

  EXPECT_EQ(0, xstream->SetConfig("config_file", "./test/configs/filter.json"));
  EXPECT_EQ(0, xstream->SetConfig("profiler", "on"));
  EXPECT_EQ(0, xstream->SetConfig("profiler_file", "./profiler.txt"));
  EXPECT_EQ(0, xstream->Init());
  ASSERT_EQ(0,
            xstream->SetCallback(std::bind(&NodeTest::Callback::OnCallback,
                                           &callback, std::placeholders::_1)));
  ASSERT_EQ(0, xstream->SetCallback(std::bind(&NodeTest::Callback::OnCallback,
                                              &callback, std::placeholders::_1),
                                    "BBoxFilter_1"));
  ASSERT_EQ(0, xstream->SetCallback(std::bind(&NodeTest::Callback::OnCallback,
                                              &callback, std::placeholders::_1),
                                    "BBoxFilter_2"));
  InputDataPtr inputdata(new InputData());
  BaseDataVector *data(new BaseDataVector);
  xstream::BBox *bbox1(
      new xstream::BBox(hobot::vision::BBox(0, 0, 60, 60)));
  bbox1->type_ = "BBox";
  data->name_ = "face_head_box";
  data->datas_.push_back(BaseDataPtr(bbox1));
  inputdata->datas_.push_back(BaseDataPtr(data));

  std::string unique_name_1 = "BBoxFilter_1";
  auto ipp_1 = std::make_shared<xstream::FilterParam>(unique_name_1);
  ipp_1->SetThreshold(4900.0);
  EXPECT_EQ(xstream->UpdateConfig(ipp_1->unique_name_, ipp_1), 0);
  inputdata->params_.clear();
  xstream::InputParamPtr pass_through(new xstream::DisableParam(
      "BBoxFilter_1", xstream::DisableParam::Mode::PassThrough));
  inputdata->params_.push_back(pass_through);

  auto out = xstream->SyncPredict(inputdata);
  callback.OnCallback(out);
  for (auto data : out->datas_) {
    if (data->error_code_ < 0) {
      std::cout << "data error: " << data->error_code_ << std::endl;
      continue;
    }
    BaseDataVector *pdata = reinterpret_cast<BaseDataVector *>(data.get());
    EXPECT_EQ(pdata->datas_.size(), uint(1));
  }

  delete xstream;
}

TEST(Interface, UsePreDefine) {
  auto xstream = xstream::XStreamSDK::CreateSDK();
  ASSERT_TRUE(xstream);
  NodeTest::Callback callback;

  EXPECT_EQ(0, xstream->SetConfig("config_file", "./test/configs/filter.json"));
  EXPECT_EQ(0, xstream->Init());
  ASSERT_EQ(0,
            xstream->SetCallback(std::bind(&NodeTest::Callback::OnCallback,
                                           &callback, std::placeholders::_1)));
  ASSERT_EQ(0, xstream->SetCallback(std::bind(&NodeTest::Callback::OnCallback,
                                              &callback, std::placeholders::_1),
                                    "BBoxFilter_2"));
  InputDataPtr inputdata(new InputData());
  BaseDataVector *data(new BaseDataVector);
  xstream::BBox *bbox1(
      new xstream::BBox(hobot::vision::BBox(0, 0, 60, 60)));
  bbox1->type_ = "BBox";
  data->name_ = "face_head_box";
  data->datas_.push_back(BaseDataPtr(bbox1));
  inputdata->datas_.push_back(BaseDataPtr(data));

  inputdata->params_.clear();
  xstream::DisableParamPtr pre_define(new xstream::DisableParam(
      "BBoxFilter_1", xstream::DisableParam::Mode::UsePreDefine));
  BaseDataVector *pre_data(new BaseDataVector);
  xstream::BBox *pre_bbox1(
      new xstream::BBox(hobot::vision::BBox(0, 0, 70, 70)));
  pre_bbox1->type_ = "BBox";
  pre_data->name_ = "face_head_box";
  pre_data->datas_.push_back(BaseDataPtr(pre_bbox1));
  std::cout << "UsePreDefine is : 0, 0, 70, 70" << std::endl;

  pre_define->pre_datas_.emplace_back(BaseDataPtr(pre_data));

  inputdata->params_.push_back(pre_define);
  auto out = xstream->SyncPredict(inputdata);
  callback.OnCallback(out);
  for (auto data : out->datas_) {
    if (data->error_code_ < 0) {
      std::cout << "data error: " << data->error_code_ << std::endl;
      continue;
    }
    BaseDataVector *pdata = reinterpret_cast<BaseDataVector *>(data.get());
    EXPECT_EQ(pdata->datas_.size(), uint(1));
    std::cout << pdata->type_ << std::endl;
    auto in_rects =
        std::static_pointer_cast<BaseDataVector>(out->datas_.front());
    for (auto &in_rect : in_rects->datas_) {
      assert("BBox" == in_rect->type_);
      auto bbox = std::static_pointer_cast<xstream::BBox>(in_rect);
      EXPECT_EQ(bbox->value.x2, 70);
      EXPECT_EQ(bbox->value.y2, 70);
      std::cout << "output face_head_box: " << bbox->value.x1 << ","
                << bbox->value.y1 << "," << bbox->value.x2 << ","
                << bbox->value.y2 << std::endl;
    }
  }

  delete xstream;
}

TEST(Interface, Disable) {
  xstream::XStreamSDK *xstream = xstream::XStreamSDK::CreateSDK();
  ASSERT_TRUE(xstream);
  NodeTest::Callback callback;
  EXPECT_EQ(0,
            xstream->SetConfig("config_file",
                               "./test/configs/BBoxFilter_DisableMethod.json"));
  EXPECT_EQ(0, xstream->Init());
  InputDataPtr inputdata(new InputData());
  BaseDataVector *data(new BaseDataVector);
  xstream::BBox *bbox1(
      new xstream::BBox(hobot::vision::BBox(0, 0, 80, 80)));
  bbox1->type_ = "BBox";
  data->name_ = "face_head_box";
  data->datas_.push_back(BaseDataPtr(bbox1));
  inputdata->datas_.push_back(BaseDataPtr(data));
  xstream::InputParamPtr invalid(new xstream::DisableParam("BBoxFilter_2"));
  inputdata->params_.push_back(invalid);
  auto out = xstream->SyncPredict(inputdata);
  callback.OnCallback(out);

  for (std::size_t i = 0; i < out->datas_.size(); i++) {
    if ("face_head_box_filter_4" == out->datas_[i]->name_) {
      EXPECT_EQ(out->datas_[i]->state_, xstream::DataState::INVALID);
    }
    if ("face_head_box_filter_6" == out->datas_[i]->name_) {
      EXPECT_EQ(out->datas_[i]->state_, xstream::DataState::VALID);
    }
  }

  delete xstream;
}
