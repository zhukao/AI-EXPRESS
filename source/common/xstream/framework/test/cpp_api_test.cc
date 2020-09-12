/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file cpp_api_test.cc
 * @brief
 * @author tangji.sun
 * @email tangji.sun@horizon.ai
 * @date 2019/10/8
 */
#include <gtest/gtest.h>

#include <future>

#include "hobotxsdk/xstream_data.h"
#include "hobotxsdk/xstream_error.h"
#include "hobotxsdk/xstream_sdk.h"
#include "include/test_param.h"
#include "memory"

using PromiseType = std::promise<xstream::OutputDataPtr>;
namespace XStreamAPITest {
class Callback {
 public:
  void OnCallback(xstream::OutputDataPtr output) {
    ASSERT_TRUE(output);
    ASSERT_TRUE(output->context_);
    if (output->unique_name_.empty()) {
      auto promise = reinterpret_cast<const PromiseType *>(output->context_);
      const_cast<PromiseType *>(promise)->set_value(output);
    } else {
      tmp_result = output;
    }
  }
  xstream::OutputDataPtr tmp_result = nullptr;
};
}  // namespace XStreamAPITest

TEST(Interface, Config) {
  auto xstream = xstream::XStreamSDK::CreateSDK();
  ASSERT_TRUE(xstream);
  XStreamAPITest::Callback callback;

  EXPECT_EQ(0, xstream->SetConfig("config_file", "./test/configs/basic.json"));
  EXPECT_EQ(0, xstream->SetConfig("profiler", "on"));
  EXPECT_EQ(0, xstream->SetConfig("profiler_file", "./profiler.txt"));
  EXPECT_EQ(0, xstream->Init());
  ASSERT_EQ(
      0, xstream->SetCallback(std::bind(&XStreamAPITest::Callback::OnCallback,
                                        &callback, std::placeholders::_1)));
  ASSERT_EQ(
      0, xstream->SetCallback(std::bind(&XStreamAPITest::Callback::OnCallback,
                                        &callback, std::placeholders::_1),
                              "first_method"));
  xstream::InputDataPtr inputdata(new xstream::InputData());
  auto xstream_input_data = std::make_shared<xstream::BaseData>();
  xstream_input_data->name_ = "global_in";
  xstream_input_data->state_ = xstream::DataState::INVALID;
  inputdata->datas_.emplace_back(xstream_input_data);
  PromiseType p;
  auto f = p.get_future();
  inputdata->context_ = &p;
  std::string method_name = "first_method";
  auto ipp = std::make_shared<xstream::TestParam>(method_name);
  EXPECT_EQ(xstream->GetConfig(method_name), nullptr);
  EXPECT_EQ(xstream->UpdateConfig(method_name, ipp), 0);
  EXPECT_EQ(xstream->GetVersion(method_name), "0.0.0");
  xstream->AsyncPredict(inputdata);
  auto output = f.get();
  EXPECT_EQ(output->error_code_, 0);
  EXPECT_EQ(output->datas_.size(), inputdata->datas_.size());
  EXPECT_EQ(output->datas_.front()->state_, inputdata->datas_.front()->state_);
  delete xstream;
}
TEST(Interface, SyncPredict) {
  auto xstream = xstream::XStreamSDK::CreateSDK();
  ASSERT_TRUE(xstream);
  EXPECT_EQ(0, xstream->SetConfig("config_file", "./test/configs/basic.json"));
  EXPECT_EQ(0, xstream->Init());
  xstream::InputDataPtr inputdata(new xstream::InputData());
  auto xstream_input_data = std::make_shared<xstream::BaseData>();
  xstream_input_data->name_ = "global_in";
  xstream_input_data->state_ = xstream::DataState::INVALID;
  inputdata->datas_.emplace_back(xstream_input_data);
  auto output = xstream->SyncPredict(inputdata);
  EXPECT_EQ(output->error_code_, 0);
  EXPECT_EQ(output->datas_.size(), inputdata->datas_.size());
  EXPECT_EQ(output->datas_.front()->state_, inputdata->datas_.front()->state_);
  delete xstream;
}

TEST(Interface, DisableParam) {
  auto xstream = xstream::XStreamSDK::CreateSDK();
  ASSERT_TRUE(xstream);
  EXPECT_EQ(0, xstream->SetConfig("config_file", "./test/configs/basic.json"));
  EXPECT_EQ(0, xstream->Init());
  xstream::InputDataPtr inputdata(new xstream::InputData());
  auto xstream_input_data = std::make_shared<xstream::BaseData>();
  xstream_input_data->name_ = "global_in";
  inputdata->datas_.emplace_back(xstream_input_data);

  xstream::InputParamPtr invalid(new xstream::DisableParam("first_method"));
  inputdata->params_.push_back(invalid);
  auto output = xstream->SyncPredict(inputdata);
  EXPECT_EQ(output->error_code_, 0);
  EXPECT_EQ(output->datas_.front()->state_, xstream::DataState::INVALID);

  inputdata->params_.clear();
  xstream::InputParamPtr best_effort_pass_through(new xstream::DisableParam(
      "first_method", xstream::DisableParam::Mode::BestEffortPassThrough));
  inputdata->params_.push_back(best_effort_pass_through);
  output = xstream->SyncPredict(inputdata);
  EXPECT_EQ(output->datas_.size(), inputdata->datas_.size());

  inputdata->params_.clear();
  xstream::DisableParamPtr pass_through(new xstream::DisableParam(
      "first_method", xstream::DisableParam::Mode::PassThrough));
  inputdata->params_.push_back(pass_through);
  output = xstream->SyncPredict(inputdata);
  // EXPECT_EQ(output->datas_.size(), inputdata->datas_.size());
  EXPECT_EQ(output->error_code_, HOBOTXSTREAM_ERROR_OUTPUT_NOT_READY);

  inputdata->params_.clear();
  xstream::DisableParamPtr pre_define(new xstream::DisableParam(
      "first_method", xstream::DisableParam::Mode::UsePreDefine));
  xstream::InputDataPtr pre_data(new xstream::InputData());
  xstream_input_data = std::make_shared<xstream::BaseData>();
  xstream_input_data->name_ = "global_in";
  xstream_input_data->state_ = xstream::DataState::INVALID;
  pre_data->datas_.emplace_back(xstream_input_data);
  pre_data->params_.push_back(pre_define);
  output = xstream->SyncPredict(pre_data);
  EXPECT_EQ(output->error_code_, HOBOTXSTREAM_ERROR_OUTPUT_NOT_READY);
  // EXPECT_EQ(output->datas_.size(), inputdata->datas_.size());

  delete xstream;
}
