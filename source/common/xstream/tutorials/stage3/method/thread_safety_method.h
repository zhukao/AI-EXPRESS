/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      thread_safety_method.h
 * @brief     ThreadSafetyMethod class
 * @author    Qingpeng Liu (qingpeng.liu@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-02-24
 */

#ifndef XSTREAM_TUTORIALS_STAGE3_METHOD_THREAD_SAFETY_H_
#define XSTREAM_TUTORIALS_STAGE3_METHOD_THREAD_SAFETY_H_

#include <atomic>
#include <string>
#include <vector>
#include "hobotxstream/method.h"
#include "xstream/tutorials/stage3/orderdata.h"

namespace xstream {

class ThreadSafetyMethod : public Method {
 public:
  int Init(const std::string &config_file_path) override;

  std::vector<std::vector<BaseDataPtr>> DoProcess(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<xstream::InputParamPtr> &param) override;

  void Finalize() override;
  int UpdateParameter(InputParamPtr ptr) override;
  InputParamPtr GetParameter() const override;
  std::string GetVersion() const override;
  void OnProfilerChanged(bool on) override;
  MethodInfo GetMethodInfo() override;
 private:
  std::atomic<int> shared_safe_value_;
};

}  // namespace xstream

#endif  // XSTREAM_TUTORIALS_STAGE3_METHOD_THREAD_SAFETY_H_
