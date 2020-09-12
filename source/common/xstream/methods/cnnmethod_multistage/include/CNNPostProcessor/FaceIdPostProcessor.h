/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: FaceIdPosePostPredictor.h
 * @Brief: declaration of the FaceIdPosePostPredictor
 * @Author: ronghui.zhang
 * @Date: 2020-01-16 10:02:28
 * @Last Modified by: ronghui.zhang
 * @Last Modified time: 2020-01-16 12:01:29
 */

#ifndef INCLUDE_POSTPROCESSOR_FACEIDPOSTPROCESSOR_H_
#define INCLUDE_POSTPROCESSOR_FACEIDPOSTPROCESSOR_H_

#include <vector>
#include "CNNPostProcessor/CNNPostProcessor.h"
#include "CNNConst.h"

namespace xstream {
namespace CnnProc {
class FaceIdPostProcessor : public CNNPostProcessor {
 public:
  FaceIdPostProcessor() {}
  virtual ~FaceIdPostProcessor() {}
  std::vector<std::vector<BaseDataPtr>>
  DoProcess(const std::vector<std::vector<BaseDataPtr> > &input,
            const std::vector<xstream::InputParamPtr> &param) override;

 private:
  BaseDataPtr
    FaceFeaturePostPro(const std::vector<std::vector<int8_t>> &mxnet_outs);
};
}  // namespace CnnProc
}  // namespace xstream
#endif  // INCLUDE_POSTPROCESSOR_FACEIDPOSTPROCESSOR_H_
