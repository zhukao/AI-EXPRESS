/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: LmkPosePostPredictor.h
 * @Brief: declaration of the LmkPosePostPredictor
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-07-17 14:18:28
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-07-17 15:13:07
 */

#ifndef INCLUDE_CNNPOSTPROCESSOR_AGEGENDERPOSTPROCESSOR_H_
#define INCLUDE_CNNPOSTPROCESSOR_AGEGENDERPOSTPROCESSOR_H_

#include <vector>
#include "CNNPostProcessor/CNNPostProcessor.h"

namespace xstream {
namespace CnnProc {
class AgeGenderPostProcessor : public CNNPostProcessor {
 public:
  AgeGenderPostProcessor() {}
  virtual ~AgeGenderPostProcessor() {}
  std::vector<std::vector<BaseDataPtr> >
  DoProcess(const std::vector<std::vector<BaseDataPtr> > &input,
            const std::vector<xstream::InputParamPtr> &param) override;

 private:
  void HandleAgeGender(const std::vector<std::vector<int8_t>> &mxnet_outs,
                       std::vector<BaseDataPtr> *output);
  BaseDataPtr AgePostPro(const std::vector<int8_t> &mxnet_out);
  BaseDataPtr GenderPostPro(const std::vector<int8_t> &mxnet_out);
};
}  // namespace CnnProc
}  // namespace xstream
#endif  // INCLUDE_CNNPOSTPROCESSOR_AGEGENDERPOSTPROCESSOR_H_
