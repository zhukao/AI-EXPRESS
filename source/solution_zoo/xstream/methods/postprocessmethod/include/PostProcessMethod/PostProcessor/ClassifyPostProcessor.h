/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: ClassifyPostProcessor.h
 * @Brief: declaration of the ClassifyPostProcessor
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-30 15:17:48
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-30 18:57:24
 */

#ifndef POSTPROCESSMETHOD_POSTPROCESSOR_CLASSIFYPOSTPROCESSOR_H_
#define POSTPROCESSMETHOD_POSTPROCESSOR_CLASSIFYPOSTPROCESSOR_H_

#include <vector>
#include <string>
#include <mutex>
#include <memory>
#include "PostProcessMethod/PostProcessor/PostProcessor.h"
#include "hobotxsdk/xstream_data.h"
#include "bpu_predict/bpu_predict.h"
#include "bpu_predict/bpu_predict_extension.h"

namespace xstream {

class ClassifyPostProcessor : public PostProcessor {
 public:
  ClassifyPostProcessor() {}
  virtual ~ClassifyPostProcessor() {}

  // virtual int Init(const std::string &cfg);

  std::vector<std::vector<BaseDataPtr>> Do(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<xstream::InputParamPtr> &param) override;

 private:
  void RunSingleFrame(const std::vector<BaseDataPtr> &frame_input,
      std::vector<BaseDataPtr> &frame_output);
};
}  // namespace xstream

#endif  // POSTPROCESSMETHOD_POSTPROCESSOR_CLASSIFYPOSTPROCESSOR_H_
