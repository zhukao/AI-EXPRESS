/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: PredictorFactory.cc
 * @Brief: definition of the PredictorFactory
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-22 14:28:17
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-22 16:19:08
 */

#ifndef PREDICTMETHOD_PREDICTOR_PREDICTORFACTORY_H_
#define PREDICTMETHOD_PREDICTOR_PREDICTORFACTORY_H_

#include "PredictMethod/Predictors/Predictors.h"
#include "PredictMethod/Predictors/DetectPredictor.h"
#include "PredictMethod/Predictors/ClassifyPredictor.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

class PredictorFactory {
 public:
  static Predictors* GetPredictor(PredictType input_type) {
    switch (input_type) {
      case PredictType::DETECT:
        return new DetectPredictor();
      case PredictType::CLASSIFY:
        return new ClassifyPredictor();
      default: {
         HOBOT_CHECK(false) << "Invalid predictor type";
         return nullptr;
      }
    }
    return nullptr;
  }
};

}  // namespace xstream
#endif  // PREDICTMETHOD_PREDICTOR_PREDICTORFACTORY_H_
