/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: VehicleColorPostPredictor.h
 * @Brief: declaration of the VehicleColorPostPredictor
 * @Author: yatao.fu
 * @Email: yatao.fu@horizon.ai
 * @Date: 2019-08-28 16:18:28
 * @Last Modified by: yatao.fu
 * @Last Modified time: 16:18:28
 */

#ifndef CNNMETHOD_VEHICLECOLOR_H
#define CNNMETHOD_VEHICLECOLOR_H

#include <memory>
#include <vector>
#include "CNNMethod/PostPredictor/PostPredictor.h"

namespace xstream {

class VehicleColorPostPredictor : public PostPredictor {
 public:
  virtual void Do(CNNMethodRunData *run_data);
  virtual int32_t Init(std::shared_ptr<CNNMethodConfig> config);

 private:
  int VehicleTColorPro(const std::vector<int8_t> &mxnet_out);
};

}  // namespace xstream

#endif  // CNNMETHOD_VEHICLECOLOR_H
