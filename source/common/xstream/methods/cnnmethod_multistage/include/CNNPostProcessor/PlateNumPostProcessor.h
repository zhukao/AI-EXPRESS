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

#ifndef CNNMETHOD_PLATENUMPOSTPROCESSOR_H
#define CNNMETHOD_PLATENUMPOSTPROCESSOR_H
#include <memory>
#include <string>
#include <vector>
#include "CNNPostProcessor/CNNPostProcessor.h"

namespace xstream {
namespace CnnProc {
class PlateNumPostProcessor : public CNNPostProcessor {
 public:
  PlateNumPostProcessor() {}
  virtual ~PlateNumPostProcessor() {}
  int Init(const std::string &cfg_path) override;
  std::vector<std::vector<BaseDataPtr>>
  DoProcess(const std::vector<std::vector<BaseDataPtr> > &input,
            const std::vector<xstream::InputParamPtr> &param) override;

 private:
  std::string PlateNumPro(const std::vector<int8_t> &mxnet_out);
  void PlateNumPro(const std::vector<int8_t> &mxnet_outs,
                   std::vector<int> &platenum);
};
}  // namespace CnnProc
}  // namespace xstream
#endif  // CNNMETHOD_PLATENUMPOSTPROCESSOR_H
