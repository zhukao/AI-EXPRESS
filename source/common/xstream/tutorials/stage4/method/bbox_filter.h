/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @brief     BBoxFilter Method
 * @author    wenhao.zou
 * @email     wenhao.zou@horizon.ai
 * @version   0.0.0.1
 * @date      2020.2.12
 */

#ifndef XSTREAM_TUTORIALS_STAGE4_METHOD_BBOX_FILTER_H_
#define XSTREAM_TUTORIALS_STAGE4_METHOD_BBOX_FILTER_H_

#include <atomic>
#include <string>
#include <vector>
#include "method/filter_param.h"
#include "hobotxstream/method.h"
#include "json/json.h"

namespace xstream {

class BBoxFilter : public Method {
 public:
  int Init(const std::string &config_file_path) override;

  std::vector<std::vector<BaseDataPtr>>
  DoProcess(const std::vector<std::vector<BaseDataPtr>> &input,
            const std::vector<xstream::InputParamPtr> &param) override;

  void Finalize() override;

  int UpdateParameter(InputParamPtr ptr) override;

  InputParamPtr GetParameter() const override;

  std::string GetVersion() const override;
  void OnProfilerChanged(bool on) override;
 private:
  std::atomic<float> area_threshold_;

  // for thread model test
  bool has_thread_params_{false};
  std::string thr_policy_;
  int thr_priority_;
};

}  // namespace xstream

#endif  // XSTREAM_TUTORIALS_STAGE4_METHOD_BBOX_FILTER_H_
