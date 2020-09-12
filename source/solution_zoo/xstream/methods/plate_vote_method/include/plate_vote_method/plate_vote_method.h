/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @brief:
 * @author : tangji.sun
 * @email : tangji.sun@horizon.ai
 * @date: 2019-11-04
 */

#ifndef INCLUDE_PLATE_VOTE_METHOD_VOTE_METHOD_H
#define INCLUDE_PLATE_VOTE_METHOD_VOTE_METHOD_H

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <utility>

#include "hobotxstream/method.h"
#include "horizon/vision_type/vision_type.hpp"

namespace xstream {

class PlateVoteParam : public xstream::InputParam {
 public:
  explicit PlateVoteParam(const std::string &module_name)
      : xstream::InputParam(module_name) {}
  std::string Format() override { return ""; }
};

using PlateVoteParamPtr = std::shared_ptr<PlateVoteParam>;

class PlateVoteMethod : public Method {
 public:
  PlateVoteMethod() : Method() {}
  virtual ~PlateVoteMethod() {}
  // return 0 for successed, -1 for failed.
  int Init(const std::string &config_file_path) override;

  std::vector<std::vector<BaseDataPtr>> DoProcess(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<xstream::InputParamPtr> &param) override;

  void RunSingleFrame(const std::vector<BaseDataPtr> &frame_input,
                      std::vector<BaseDataPtr> &frame_output);

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

  std::string Rover(const std::vector<std::string> &strs);

 private:
  int max_slide_window_size_;
  PlateVoteParamPtr method_param_;

  std::unordered_map<uint32_t,
                     std::deque<xstream::XStreamData<std::vector<int>>>>
      slide_window_map;

  std::vector<std::vector<int>> EditDistance(const std::string &src,
                                             const std::string &dest);
  std::pair<std::string, std::string> Reconstruct(
      const std::string &src, const std::string &dest,
      const std::vector<std::vector<int>> &edit);
};

}  // namespace xstream

#endif  // INCLUDE_PLATE_VOTE_METHOD_VOTE_METHOD_H
