/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: FaceQualityPosePostProcessor.h
 * @Brief: declaration of the FaceQualityPosePostProcessor
 * @Author: ronghui.zhang
 * @Date: 2020-01-20 10:02:28
 * @Last Modified by: ronghui.zhang
 * @Last Modified time: 2020-01-20 12:01:29
 */

#ifndef INCLUDE_POSTPROCESSOR_FACEQUALITYPOSTPROCESSOR_H_
#define INCLUDE_POSTPROCESSOR_FACEQUALITYPOSTPROCESSOR_H_

#include <vector>
#include <string>
#include "CNNPostProcessor/CNNPostProcessor.h"
#include "CNNConst.h"

namespace xstream {
namespace CnnProc {
class FaceQualityPostProcessor : public CNNPostProcessor {
 public:
  FaceQualityPostProcessor() {}
  virtual ~FaceQualityPostProcessor() {}
  int Init(const std::string &cfg_path) override;
  int UpdateParameter(xstream::InputParamPtr ptr) override;

  std::vector<std::vector<BaseDataPtr>>
  DoProcess(const std::vector<std::vector<BaseDataPtr> > &input,
            const std::vector<xstream::InputParamPtr> &param) override;


 private:
  void FaceQualityPostPro(const std::vector<std::vector<int8_t>> &mxnet_outs,
                          std::vector<BaseDataPtr> *output);
  float threshold_ = 0.0f;
};
}  // namespace CnnProc
}  // namespace xstream
#endif  // INCLUDE_POSTPROCESSOR_FACEIDPOSTPROCESSOR_H_
