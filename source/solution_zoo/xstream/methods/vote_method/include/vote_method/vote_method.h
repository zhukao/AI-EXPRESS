/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @brief: vote method
 * @author : tangji.sun
 * @email : tangji.sun@horizon.ai
 * @date: 2019-11-04
 */

#ifndef INCLUDE_VOTE_METHOD_VOTE_METHOD_H
#define INCLUDE_VOTE_METHOD_VOTE_METHOD_H

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

#include "hobotxstream/method.h"
#include "horizon/vision_type/vision_type.hpp"

namespace xstream {

class VoteParam : public xstream::InputParam {
 public:
  explicit VoteParam(const std::string &module_name)
    : xstream::InputParam(module_name) {}
  std::string Format() override {
    return "";
  }
};

using VoteParamPtr = std::shared_ptr<VoteParam>;

class VoteMethod : public Method {
 public:
  VoteMethod() : Method() {}
  virtual ~VoteMethod() {}
  // return 0 for successed, -1 for failed.
  int Init(const std::string &config_file_path) override;

  std::vector<std::vector<BaseDataPtr>> DoProcess(
              const std::vector<std::vector<BaseDataPtr>> &input,
              const std::vector<xstream::InputParamPtr> &param) override;

  void RunSingleFrame(const std::vector<BaseDataPtr> &frame_input,
                      std::vector<BaseDataPtr> &frame_output);

  void Finalize() override {}

  int UpdateParameter(InputParamPtr ptr) override {
    return 0;
  }

  InputParamPtr GetParameter() const override {
    return nullptr;
  }

  std::string GetVersion() const override {
    return "";
  }

  void OnProfilerChanged(bool on) override {}

  MethodInfo GetMethodInfo() override {
    MethodInfo method_info;
    method_info.is_thread_safe_ = false;
    method_info.is_need_reorder = true;
    return method_info;
  };


 private:
  void AdjustQueue(const XStreamData<int> &vote_info,
                  uint32_t track_id,
                  float score,
                  float timestamp);
  void Vote(std::shared_ptr<XStreamData<int>> &vote_info_ptr,
            uint32_t track_id);

 private:
  int max_slide_window_size_;
  VoteParamPtr method_param_;

  float positive_voting_threshold_;
  float negative_voting_threshold_;
  float time_interval_;

  std::unordered_map<uint32_t, std::deque<xstream::XStreamData<int>>>
  slide_window_map;
  int type_;  // 0 vehicle type & color ,1 plate_color , 2 living & behavior
  std::unordered_map<uint32_t, float> score_;  // plate_color need
  std::unordered_map<uint32_t, std::deque<float>> timestamp_map_;
};

}  // namespace xstream

#endif  // INCLUDE_VOTE_METHOD_VOTE_METHOD_H
