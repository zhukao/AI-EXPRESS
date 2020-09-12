/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     BBoxFilter Method
 * @author    shuhuan.sun
 * @email     shuhuan.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2018.11.23
 */

#ifndef FILTER_SKIP_FRAME_METHOD_H_
#define FILTER_SKIP_FRAME_METHOD_H_

#include <atomic>
#include <string>
#include <unordered_map>
#include <vector>
#include <deque>

// #include "filter_param.h"
#include "hobotxstream/method.h"
#include "horizon/vision_type/vision_type.hpp"

namespace xstream {

class FilterSkipFrameParam : public InputParam {
 public:
  explicit FilterSkipFrameParam(const std::string &module_name);
  std::string Format() override;

 public:
  std::string config_str;
};

class FilterSkipFrameMethod : public Method {
 public:
  FilterSkipFrameMethod();

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
  /**
   * @brief 更新配置
   *
   * @param config_str
   *
   * @return 0 success; other failed
   */
  int UpdateConfig(const std::string &config_str);

 private:
  // std::atomic<float> area_threshold_;
  std::string config_file_name_;
  std::atomic<int> img_width_;
  std::atomic<int> img_height_;
  std::atomic<int> min_width_;
  std::atomic<int> min_height_;
  std::atomic<int> border_;
  std::atomic<int> skip_num_;
  std::atomic<float> min_score_;
  std::unordered_map<uint32_t, int32_t> trackState_;
  std::deque<uint32_t> trackVec_;
};

}  // namespace xstream

#endif  // FILTER_SKIP_FRAME_METHOD_H_
