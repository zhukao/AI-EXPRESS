/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: VehicleTypePostPredictor.h
 * @Brief: declaration of the VehicleTypePostPredictor
 * @Author: yatao.fu
 * @Email: yatao.fu@horizon.ai
 * @Date: 2019-08-28 16:18:28
 * @Last Modified by: yatao.fu
 * @Last Modified time: 16:18:28
 */

#ifndef CNNMETHOD_VEHICLETYPE_H
#define CNNMETHOD_VEHICLETYPE_H

#include <memory>
#include <vector>
#include <string>
#include "CNNPostProcessor/CNNPostProcessor.h"
using hobot::vision::BBox;
namespace xstream {
namespace CnnProc {
class VehicleTypePostProcessor : public CNNPostProcessor {
 public:
  VehicleTypePostProcessor() {}
  virtual ~VehicleTypePostProcessor() {}
  int Init(const std::string &cfg_path) override;
  std::vector<std::vector<BaseDataPtr>>
  DoProcess(const std::vector<std::vector<BaseDataPtr> > &input,
            const std::vector<xstream::InputParamPtr> &param) override;
 private:
  int VehicleTypePro(const std::vector<int8_t> &mxnet_out);
};
}  // namespace CnnProc
}  // namespace xstream
#endif  // CNNMETHOD_VEHICLETYPE_H
