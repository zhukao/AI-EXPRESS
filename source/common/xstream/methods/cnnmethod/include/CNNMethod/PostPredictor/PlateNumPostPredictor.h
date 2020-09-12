/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: VehicleType.h
 * @Brief: declaration of the PlateNumPostPredictor
 * @Author: yatao.fu
 * @Email: yatao.fu@horizon.ai
 * @Date: 2019-09-28 16:18:28
 * @Last Modified by: yatao.fu
 * @Last Modified time: 16:18:28
 */

#ifndef CNNMETHOD_PLATENUMPOSTPREDICTOR_H
#define CNNMETHOD_PLATENUMPOSTPREDICTOR_H
#include <memory>
#include <string>
#include <vector>
#include "CNNMethod/PostPredictor/PostPredictor.h"

namespace xstream {

class PlateNumPostPredictor : public PostPredictor {
 public:
  virtual void Do(CNNMethodRunData *run_data);
  virtual int32_t Init(std::shared_ptr<CNNMethodConfig> config);

 private:
  std::string PlateNumPro(const std::vector<int8_t> &mxnet_out);
  void PlateNumPro(const std::vector<int8_t> &mxnet_outs,
                   std::vector<int> &platenum);
};
}  // namespace xstream

#endif  // CNNMETHOD_PLATENUMPOSTPREDICTOR_H
