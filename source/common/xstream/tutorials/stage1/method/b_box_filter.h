/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      b_box_filter.h
 * @brief     BBoxFilter class
 * @author    Qingpeng Liu (qingpeng.liu@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-01-03
 */

#ifndef XSTREAM_TUTORIALS_STAGE1_METHOD_B_BOX_FILTER_H_
#define XSTREAM_TUTORIALS_STAGE1_METHOD_B_BOX_FILTER_H_

#include <atomic>
#include <string>
#include <vector>
#include "hobotxstream/method.h"
#include "xstream/tutorials/stage1/filter_param.h"

namespace xstream {

class BBoxFilter : public Method {
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

 private:
  std::atomic<float> area_threshold_;
};

}  // namespace xstream

#endif  // XSTREAM_TUTORIALS_STAGE1_METHOD_B_BOX_FILTER_H_
