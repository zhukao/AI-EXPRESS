/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file xstream_test.cc
 * @brief
 * @author shiqing.xie
 * @email shiqing.xie@horizon.ai
 * @date 2019/11/21
 */
#include <gtest/gtest.h>

#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "hobotxsdk/xstream_data.h"
#include "hobotxsdk/xstream_error.h"
#include "hobotxsdk/xstream_sdk.h"
#include "include/test_param.h"
#include "memory"

using PromiseType = std::promise<xstream::OutputDataPtr>;
namespace xstream_test {
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
}  //  namespace xstream_test

TEST(Interface, UpdataConfig) {
  auto xstream = xstream::XStreamSDK::CreateSDK();
  ASSERT_TRUE(xstream);
  xstream_test::Callback callback;

  EXPECT_EQ(
      0, xstream->SetConfig("config_file", "./test/configs/UpdataConfig.json"));
  EXPECT_EQ(0, xstream->SetConfig("profiler", "on"));
  EXPECT_EQ(0, xstream->SetConfig("profiler_file", "./profiler.txt"));
  EXPECT_EQ(0, xstream->Init());
  ASSERT_EQ(0,
            xstream->SetCallback(std::bind(&xstream_test::Callback::OnCallback,
                                           &callback, std::placeholders::_1)));
  ASSERT_EQ(0,
            xstream->SetCallback(std::bind(&xstream_test::Callback::OnCallback,
                                           &callback, std::placeholders::_1),
                                 "method_1"));
  xstream::InputDataPtr inputdata(new xstream::InputData());
  auto xstream_input_data = std::make_shared<xstream::BaseData>();
  xstream_input_data->name_ = "input_in";
  xstream_input_data->state_ = xstream::DataState::INVALID;
  inputdata->datas_.emplace_back(xstream_input_data);
  PromiseType p;
  auto f = p.get_future();
  inputdata->context_ = &p;
  std::string method_name = "method_1";
  auto ipp = std::make_shared<xstream::TestParam>(method_name);
  EXPECT_EQ(xstream->UpdateConfig(method_name, ipp), 0);
  xstream->AsyncPredict(inputdata);
  auto output = f.get();
  EXPECT_EQ(output->error_code_, 0);
  EXPECT_EQ(output->datas_.size(), inputdata->datas_.size());
  EXPECT_EQ(output->datas_.front()->state_, inputdata->datas_.front()->state_);
  delete xstream;
}

/* 确保不调用Init接口的情况下，XStreamSDK也可以正常析构 */
TEST(Interface, SingleAPI_CreateSDK) {
  bool is_normal = true;
  {
    auto sdk =
        std::shared_ptr<xstream::XStreamSDK>(xstream::XStreamSDK::CreateSDK());
  }

  ASSERT_TRUE(is_normal);
}

TEST(Interface, ReCreateSDK) {
  for (int i = 0; i < 10; ++i) {
    auto xstream = xstream::XStreamSDK::CreateSDK();
    ASSERT_TRUE(xstream);
    xstream_test::Callback callback;
    EXPECT_EQ(0, xstream->SetConfig("config_file",
                                    "./test/configs/UpdataConfig.json"));
    EXPECT_EQ(0, xstream->Init());
    xstream::InputDataPtr inputdata(new xstream::InputData());
    auto xstream_input_data = std::make_shared<xstream::BaseData>();
    xstream_input_data->name_ = "input_in";
    xstream_input_data->state_ = xstream::DataState::VALID;
    inputdata->datas_.emplace_back(xstream_input_data);
    PromiseType p;
    auto f = p.get_future();
    inputdata->context_ = &p;
    auto out = xstream->SyncPredict(inputdata);
    callback.OnCallback(out);
    delete xstream;
  }
}

TEST(Interface, MultiSDK) {
  auto xstream = xstream::XStreamSDK::CreateSDK();
  auto xstream_2 = xstream::XStreamSDK::CreateSDK();
  ASSERT_TRUE(xstream);
  ASSERT_TRUE(xstream_2);
  xstream_test::Callback callback;

  EXPECT_EQ(
      0, xstream->SetConfig("config_file", "./test/configs/UpdataConfig.json"));
  EXPECT_EQ(0, xstream->SetConfig("profiler", "on"));
  EXPECT_EQ(0, xstream->Init());

  EXPECT_EQ(0, xstream_2->SetConfig("config_file",
                                    "./test/configs/UpdataConfig.json"));
  EXPECT_EQ(0, xstream_2->SetConfig("profiler", "on"));
  EXPECT_EQ(0, xstream_2->Init());

  ASSERT_EQ(0,
            xstream->SetCallback(std::bind(&xstream_test::Callback::OnCallback,
                                           &callback, std::placeholders::_1)));
  ASSERT_EQ(0,
            xstream->SetCallback(std::bind(&xstream_test::Callback::OnCallback,
                                           &callback, std::placeholders::_1),
                                 "method_1"));
  ASSERT_EQ(0,
            xstream->SetCallback(std::bind(&xstream_test::Callback::OnCallback,
                                           &callback, std::placeholders::_1),
                                 "method_2"));
  ASSERT_EQ(0,
            xstream->SetCallback(std::bind(&xstream_test::Callback::OnCallback,
                                           &callback, std::placeholders::_1),
                                 "method_3"));

  xstream::InputDataPtr inputdata(new xstream::InputData());
  auto xstream_input_data = std::make_shared<xstream::BaseData>();
  xstream_input_data->name_ = "input_in";
  xstream_input_data->state_ = xstream::DataState::VALID;
  inputdata->datas_.emplace_back(xstream_input_data);
  PromiseType p;
  auto f = p.get_future();
  inputdata->context_ = &p;
  std::string method_name = "method_1";
  auto ipp = std::make_shared<xstream::TestParam>(method_name);
  EXPECT_EQ(xstream->UpdateConfig(method_name, ipp), 0);
  std::string method_name_2 = "method_2";
  auto ipp_2 = std::make_shared<xstream::TestParam>(method_name_2);
  EXPECT_EQ(xstream->UpdateConfig(method_name_2, ipp_2), 0);
  std::string method_name_3 = "method_3";
  auto ipp_3 = std::make_shared<xstream::TestParam>(method_name_3);
  EXPECT_EQ(xstream->UpdateConfig(method_name_3, ipp_3), 0);

  xstream->AsyncPredict(inputdata);
  auto output = f.get();
  EXPECT_EQ(output->error_code_, 0);
  EXPECT_EQ(output->datas_.size(), inputdata->datas_.size());
  EXPECT_EQ(output->datas_.front()->state_, inputdata->datas_.front()->state_);

  PromiseType p1;
  auto f1 = p1.get_future();
  inputdata->context_ = &p1;
  auto ipp_SyncPredict = std::make_shared<xstream::TestParam>(method_name);
  EXPECT_EQ(xstream_2->UpdateConfig(method_name, ipp_SyncPredict), 0);
  auto out = xstream_2->SyncPredict(inputdata);
  callback.OnCallback(out);

  delete xstream;
  delete xstream_2;
}

void TestSDK(int IterNum, const std::string &config) {
  for (int i = 0; i < 100; i++) {
    std::cout << "Start running the:[" << IterNum + 1
              << "] Thread and start create the:[" << i + 1
              << "] xstream-framework-SDK\n"
              << std::endl;
    auto xstream = xstream::XStreamSDK::CreateSDK();
    ASSERT_TRUE(xstream);
    xstream_test::Callback callback;
    EXPECT_EQ(0, xstream->SetConfig("config_file", config));
    EXPECT_EQ(0, xstream->Init());
    xstream::InputDataPtr inputdata(new xstream::InputData());
    auto xstream_input_data = std::make_shared<xstream::BaseData>();
    xstream_input_data->name_ = "input_in";
    xstream_input_data->state_ = xstream::DataState::VALID;
    inputdata->datas_.emplace_back(xstream_input_data);
    PromiseType p;
    auto f = p.get_future();
    inputdata->context_ = &p;
    auto out = xstream->SyncPredict(inputdata);
    callback.OnCallback(out);
    EXPECT_EQ(out->datas_.front()->state_, xstream::DataState::VALID);
    delete xstream;
  }
}

int MulTHDCreateSDKTest(const std::string &config) {
  std::cout << "MulTHDCreateSDKTest" << std::endl;
  std::vector<std::thread> testthread;
  for (int i = 0; i < 20; i++) {
    testthread.push_back(std::thread(TestSDK, i, config));
  }
  for (auto iter = testthread.begin(); iter != testthread.end(); ++iter) {
    iter->join();
  }

  return 0;
}

TEST(Interface, MulTHDCreateSDK) {
  MulTHDCreateSDKTest("./test/configs/UpdataConfig.json");
}
