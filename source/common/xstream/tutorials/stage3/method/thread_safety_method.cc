/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      thread_safety_method.cc
 * @brief     ThreadSafetyMethod class implementation
 * @author    Qingpeng Liu (qingpeng.liu@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-02-24
 */

#include "xstream/tutorials/stage3/method/thread_safety_method.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <thread>
#include "xstream/tutorials/stage3/orderdata.h"
#include "hobotxstream/profiler.h"
#include "hobotxsdk/xstream_data.h"
#include "json/json.h"

namespace xstream {

int ThreadSafetyMethod::Init(const std::string &config_file_path) {
  std::cout << "ThreadSafetyMethod::Init " << config_file_path << std::endl;
  shared_safe_value_ = 0;
  return 0;
}

int ThreadSafetyMethod::UpdateParameter(InputParamPtr ptr) {
  return 0;
}

InputParamPtr ThreadSafetyMethod::GetParameter() const {
  return InputParamPtr();
}

std::vector<std::vector<BaseDataPtr>> ThreadSafetyMethod::DoProcess(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<InputParamPtr> &param) {
  std::cout << "ThreadSafetyMethod::DoProcess" << std::endl;
  shared_safe_value_.fetch_add(1);
  std::vector<std::vector<BaseDataPtr>> output;
  output.resize(input.size());
  for (size_t i = 0; i < input.size(); ++i) {
    // 当前不支持batch模式，batch恒等于1
    assert(i <= 1);
    auto &in_batch_i = input[i];
    auto &out_batch_i = output[i];
    out_batch_i.resize(in_batch_i.size());
    std::cout << "input size: " << in_batch_i.size() << std::endl;
    // 只支持n个输入，输入格式是BBox的数组
    for (size_t j = 0; j < in_batch_i.size(); ++j) {
      auto in_rects = std::static_pointer_cast<BaseDataVector>(in_batch_i[j]);
      assert("BaseDataVector" == in_rects->type_);
      auto out_rects = std::make_shared<BaseDataVector>();
      out_batch_i[j] = std::static_pointer_cast<BaseData>(out_rects);
      for (auto &in_rect : in_rects->datas_) {
        auto safe_data =
          std::static_pointer_cast<xstream::OrderData>(in_rect);
        safe_data->sequence_id = shared_safe_value_;
        std::cout << "thread safety method safe_data sequence_id:"
                  << safe_data->sequence_id << std::endl;
        out_rects->datas_.push_back(in_rect);
      }
    }
  }
  return output;
}

void ThreadSafetyMethod::Finalize() {
  std::cout << "ThreadSafetyMethod::Finalize" << std::endl;
}

std::string ThreadSafetyMethod::GetVersion() const { return "test_only"; }

void ThreadSafetyMethod::OnProfilerChanged(bool on) {}

MethodInfo ThreadSafetyMethod::GetMethodInfo() {
  MethodInfo order_method = MethodInfo();
  order_method.is_thread_safe_ = true;
  order_method.is_need_reorder = true;
  return order_method;
}

}  // namespace xstream
