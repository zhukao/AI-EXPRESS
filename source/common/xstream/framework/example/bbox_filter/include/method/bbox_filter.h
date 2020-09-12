/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     BBoxFilter Method
 * @author    shuhuan.sun
 * @email     shuhuan.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2018.11.23
 */

#ifndef METHOD_BBOX_FILTER_H_
#define METHOD_BBOX_FILTER_H_

#include <atomic>
#include <string>
#include <vector>
#include "hobotxstream/data_types/filter_param.h"
#include "hobotxstream/method.h"

namespace xstream {

class BBoxFilter : public Method {
 public:
  int Init(const std::string &config_file_path) override;

  std::vector<std::vector<BaseDataPtr>> DoProcess(const std::vector<std::vector<BaseDataPtr>> &input,
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

#endif  // METHOD_BBOX_FILTER_H_
