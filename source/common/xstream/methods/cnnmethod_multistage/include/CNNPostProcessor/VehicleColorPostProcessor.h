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
#include <string>
#include "CNNPostProcessor/CNNPostProcessor.h"

namespace xstream {
namespace CnnProc {
class VehicleColorPostProcessor : public CNNPostProcessor {
 public:
  VehicleColorPostProcessor() {}
  virtual ~VehicleColorPostProcessor() {}
  int Init(const std::string &cfg_path) override;
  std::vector<std::vector<BaseDataPtr>>
  DoProcess(const std::vector<std::vector<BaseDataPtr> > &input,
            const std::vector<xstream::InputParamPtr> &param) override;

 private:
  int VehicleTColorPro(const std::vector<int8_t> &mxnet_out);
};
}  // namespace CnnProc
}  // namespace xstream
#endif  // CNNMETHOD_VEHICLECOLOR_H
