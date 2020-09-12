/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @Author: xudong.du
 * @Email: xudong.du@horizon.ai
 * @Date: 2020-05-16 14:18:28
 * @Last Modified by: xudong.du
 * @Last Modified time: 2020-06-06 15:18:28
 */
#ifndef INCLUDE_CNNMETHOD_PREDICTOR_VID_H_
#define INCLUDE_CNNMETHOD_PREDICTOR_VID_H_
#include <thread>

#include "CNNMethod/Predictor/Predictor.h"
#include "CNNMethod/util/CNNMethodData.h"

namespace xstream {

class VidInputPredictor : public Predictor {
 public:
  VidInputPredictor();
  void Do(CNNMethodRunData *run_data) override;
  ~VidInputPredictor() override = default;
};
}  // namespace xstream
#endif  // INCLUDE_CNNMETHOD_PREDICTOR_VID_H_
