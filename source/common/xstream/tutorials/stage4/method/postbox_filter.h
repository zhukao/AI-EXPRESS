/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     BBoxFilter Method
 * @author    shuhuan.sun
 * @email     shuhuan.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2018.11.23
 */

#ifndef XSTREAM_TUTORIALS_STAGE4_METHOD_POSTBOX_FILTER_H_
#define XSTREAM_TUTORIALS_STAGE4_METHOD_POSTBOX_FILTER_H_

#include <atomic>
#include <string>
#include <vector>
#include "method/filter_param.h"
#include "hobotxstream/method.h"
#include "json/json.h"

namespace xstream {

class PostBoxFilter : public Method {
 public:
  PostBoxFilter():Method() {
    max_height_ = 40;
    max_width_ = 40;
  }
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
  float max_width_;
  float max_height_;
  // for thread model test
  bool has_thread_params_{false};
  std::string thr_policy_;
  int thr_priority_;
};

}  // namespace xstream

#endif  // XSTREAM_TUTORIALS_STAGE4_METHOD_POSTBOX_FILTER_H_
