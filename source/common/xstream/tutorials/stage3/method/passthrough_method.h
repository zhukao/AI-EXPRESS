/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      passthrough_method.h
 * @brief     PassthroughMethod class
 * @author    Qingpeng Liu (qingpeng.liu@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-02-24
 */

#ifndef XSTREAM_TUTORIALS_STAGE3_METHOD_PASSTHROUGH_H_
#define XSTREAM_TUTORIALS_STAGE3_METHOD_PASSTHROUGH_H_

#include <atomic>
#include <string>
#include <vector>
#include "hobotxstream/method.h"
#include "xstream/tutorials/stage3/orderdata.h"

namespace xstream {

class PassthroughMethod : public Method {
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
};

}  // namespace xstream

#endif  // XSTREAM_TUTORIALS_STAGE3_METHOD_PASSTHROUGH_H_
