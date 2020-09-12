/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: ClassifyPredictors.h
 * @Brief: declaration of the ClassifyPredictors
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-30 16:00:48
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-30 18:57:24
 */

#ifndef PREDICTMETHOD_PREDICTORS_CLASSIFYPREDICTOR_H_
#define PREDICTMETHOD_PREDICTORS_CLASSIFYPREDICTOR_H_

#include <vector>
#include <string>
#include <mutex>
#include <memory>
#include "PredictMethod/Predictors/Predictors.h"
#include "hobotxsdk/xstream_data.h"
#include "bpu_predict/bpu_predict.h"
#include "bpu_predict/bpu_predict_extension.h"

namespace xstream {

class ClassifyPredictor : public Predictors {
 public:
  ClassifyPredictor() {}
  virtual ~ClassifyPredictor() {}

  // virtual int Init(const std::string &cfg_path);

  std::vector<std::vector<BaseDataPtr>> Do(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<xstream::InputParamPtr> &param) override;

 private:
  std::vector<BaseDataPtr> RunSingleFrame(
      const std::vector<BaseDataPtr> &frame_input);
};
}  // namespace xstream

#endif  // PREDICTMETHOD_PREDICTORS_CLASSIFYPREDICTOR_H_
