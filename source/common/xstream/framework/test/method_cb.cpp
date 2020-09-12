/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file method_cb.cpp
 * @brief
 * @author ruoting.ding
 * @email ruoting.ding@horizon.ai
 * @date 2019/9/24
 */

#include <gtest/gtest.h>

#include <future>
#include <vector>

#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_sdk.h"
#include "memory"

using PromiseType = std::promise<xstream::OutputDataPtr>;
namespace MethodCallback {
class Callback {
 public:
  void OnCallback(xstream::OutputDataPtr output) {
    ASSERT_TRUE(output);
    ASSERT_TRUE(output->context_);
    if (output->unique_name_.empty()) {
      auto promise = reinterpret_cast<const PromiseType *>(output->context_);
      const_cast<PromiseType *>(promise)->set_value(output);
    } else {
      LOGI << "method callback, name=" << output->unique_name_;
      tmp_result = output;
    }
  }
  xstream::OutputDataPtr tmp_result = nullptr;
};
}  // namespace MethodCallback

TEST(CallBack, Global) {
  auto xstream = xstream::XStreamSDK::CreateSDK();
  ASSERT_TRUE(xstream);
  MethodCallback::Callback callback;
  EXPECT_EQ(0, xstream->SetConfig("config_file", "./test/configs/basic.json"));
  EXPECT_EQ(0, xstream->Init());
  xstream->SetCallback(std::bind(&MethodCallback::Callback::OnCallback,
                                 &callback, std::placeholders::_1));
  xstream::InputDataPtr inputdata(new xstream::InputData());
  auto xstream_input_data = std::make_shared<xstream::BaseData>();
  xstream_input_data->name_ = "global_in";
  xstream_input_data->state_ = xstream::DataState::INVALID;
  inputdata->datas_.emplace_back(xstream_input_data);
  // Call 10 times at the same time
  std::vector<PromiseType> promises;
  promises.resize(10);
  std::vector<std::future<xstream::OutputDataPtr>> futures;
  for (auto &promise : promises) {
    inputdata->context_ = &promise;
    xstream->AsyncPredict(inputdata);
    futures.emplace_back(promise.get_future());
  }

  for (auto &future : futures) {
    auto output = future.get();
    EXPECT_EQ(output->error_code_, 0);
    EXPECT_EQ(output->datas_.size(), inputdata->datas_.size());
    EXPECT_EQ(output->datas_.front()->state_,
              inputdata->datas_.front()->state_);
  }

  delete xstream;
}

TEST(CallBack, Method) {
  auto xstream = xstream::XStreamSDK::CreateSDK();
  ASSERT_TRUE(xstream);
  EXPECT_EQ(0, xstream->SetConfig("config_file", "./test/configs/basic.json"));
  EXPECT_EQ(0, xstream->Init());
  MethodCallback::Callback callback;
  xstream->SetCallback(std::bind(&MethodCallback::Callback::OnCallback,
                                 &callback, std::placeholders::_1));
  xstream->SetCallback(std::bind(&MethodCallback::Callback::OnCallback,
                                 &callback, std::placeholders::_1),
                       "first_method");
  xstream::InputDataPtr inputdata(new xstream::InputData());
  auto xstream_input_data = std::make_shared<xstream::BaseData>();
  xstream_input_data->name_ = "global_in";
  xstream_input_data->state_ = xstream::DataState::INVALID;
  inputdata->datas_.emplace_back(xstream_input_data);
  PromiseType p;
  auto f = p.get_future();
  inputdata->context_ = &p;
  xstream->AsyncPredict(inputdata);
  auto output = f.get();
  EXPECT_EQ(output->error_code_, 0);
  EXPECT_EQ(output->datas_.size(), inputdata->datas_.size());
  EXPECT_EQ(output->datas_.front()->state_, inputdata->datas_.front()->state_);
  EXPECT_TRUE(callback.tmp_result);
  if (callback.tmp_result) {
    EXPECT_NE(callback.tmp_result->error_code_, 0);
    EXPECT_EQ(callback.tmp_result->datas_.size(), uint(2));
    if (callback.tmp_result->datas_.size() == 4) {
      EXPECT_EQ(callback.tmp_result->datas_[0]->name_, "tmp_data1");
      EXPECT_EQ(callback.tmp_result->datas_[1]->name_, "tmp_data2");
    }
  }
  // unset callback
  callback.tmp_result = nullptr;
  xstream->SetCallback(nullptr, "first_method");
  PromiseType p_unset_cb;
  auto f_unset_cb = p_unset_cb.get_future();
  inputdata->context_ = &p_unset_cb;
  xstream->SetConfig("free_framedata", "on");
  xstream->AsyncPredict(inputdata);
  output = f_unset_cb.get();
  EXPECT_EQ(output->error_code_, 0);
  EXPECT_EQ(callback.tmp_result, nullptr);
  delete xstream;
}
