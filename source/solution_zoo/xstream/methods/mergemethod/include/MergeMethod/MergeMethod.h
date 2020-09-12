/*
 * @Description: implement of data_type
 * @Author: ruoting.ding@horizon.ai
 * @Date: 2019-11-15 17:49:26
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-11-20 11:48:45
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */

#ifndef INCLUDE_MERGEMETHOD_MERGEMETHOD_H_
#define INCLUDE_MERGEMETHOD_MERGEMETHOD_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "data_type/data_type.h"
#include "hobotxstream/method.h"

namespace xstream {

class MergeMethod : public Method {
 public:
  int Init(const std::string &config_file_path) override;

  std::vector<std::vector<BaseDataPtr>> DoProcess(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<xstream::InputParamPtr> &param) override;

  void Finalize() override;

  int UpdateParameter(InputParamPtr ptr) override;

  InputParamPtr GetParameter() const override;

  std::string GetVersion() const override { return "0.0.17"; }

  void OnProfilerChanged(bool on) override {}

  MethodInfo GetMethodInfo() override {
    MethodInfo method_info;
    method_info.is_thread_safe_ = true;
    return method_info;
  };

 protected:
  std::shared_ptr<JsonReader> reader_;
  std::shared_ptr<MergeParam> method_config_param_;
  std::shared_ptr<MergeParam> default_method_config_param_;

 private:
  std::vector<BaseDataPtr> ProcessOneBatch(const std::vector<BaseDataPtr> &in,
                                           const InputParamPtr &param);

  std::map<std::string, std::shared_ptr<MergeStrategy>> strategy_map_;
};

}  // namespace xstream

#endif  // INCLUDE_MERGEMETHOD_MERGEMETHOD_H_
