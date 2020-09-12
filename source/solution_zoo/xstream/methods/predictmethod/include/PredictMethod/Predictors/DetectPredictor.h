/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: DetectPredictor.h
 * @Brief: declaration of the DetectPredictor
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-24 15:17:48
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-27 18:57:24
 */

#ifndef PREDICTMETHOD_PREDICTORS_DETECTPREDICTOR_H_
#define PREDICTMETHOD_PREDICTORS_DETECTPREDICTOR_H_

#include <vector>
#include <string>
#include <mutex>
#include <memory>
#include "PredictMethod/Predictors/Predictors.h"
#include "hobotxsdk/xstream_data.h"
#include "bpu_predict/bpu_predict.h"
#include "bpu_predict/bpu_predict_extension.h"

namespace xstream {

class DetectPredictor : public Predictors {
 public:
  DetectPredictor() {}
  virtual ~DetectPredictor() {}

  // virtual int Init(const std::string &cfg_path);

  std::vector<std::vector<BaseDataPtr>> Do(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<xstream::InputParamPtr> &param) override;

 private:
  std::vector<BaseDataPtr> RunSingleFrame(
      const std::vector<BaseDataPtr> &frame_input);
};
}  // namespace xstream

#endif  // PREDICTMETHOD_PREDICTORS_DETECTPREDICTOR_H_
