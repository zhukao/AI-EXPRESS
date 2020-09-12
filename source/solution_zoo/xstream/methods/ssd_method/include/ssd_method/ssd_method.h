//
// Copyright 2019 Horizon Robotics.
//

#ifndef INCLUDE_SSDMETHOD_SSD_METHOD_H
#define INCLUDE_SSDMETHOD_SSD_METHOD_H

#include <deque>
#include <exception>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "hobotxstream/method.h"
#include "horizon/vision_type/vision_type.hpp"
#include "ssd_method/ssd_post_process_module.h"
#include "ssd_method/ssd_predictor.h"

namespace xstream {

class SSDParam : public xstream::InputParam {
 public:
  explicit SSDParam(const std::string &module_name)
      : xstream::InputParam(module_name) {}
  std::string Format() override { return ""; }
};

using SSDParamPtr = std::shared_ptr<SSDParam>;

class SSDMethod : public Method {
 public:
  SSDMethod() : Method() {}

  virtual ~SSDMethod() {}

  // return 0 for successed, -1 for failed.
  int Init(const std::string &config_file_path) override;

  std::vector<std::vector<BaseDataPtr>> DoProcess(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<xstream::InputParamPtr> &param) override;

  void Finalize() override {}

  int UpdateParameter(InputParamPtr ptr) override { return 0; }

  InputParamPtr GetParameter() const override { return nullptr; }

  std::string GetVersion() const override { return ""; }

  void OnProfilerChanged(bool on) override {}

  MethodInfo GetMethodInfo() override {
    MethodInfo method_info;
    method_info.is_thread_safe_ = false;
    method_info.is_need_reorder = true;
    return method_info;
  };

 private:
  std::shared_ptr<xstream::SSD_Predictor> ssd_predictor_;
  SSDParamPtr method_param_;
};

}  // namespace xstream
#endif  // INCLUDE_SSDMETHOD_SSD_METHOD_H
