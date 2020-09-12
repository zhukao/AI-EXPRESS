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

#ifndef INCLUDE_CNNPOSTPROCESSOR_ANTISPFPOSTPROCESSOR_H_
#define INCLUDE_CNNPOSTPROCESSOR_ANTISPFPOSTPROCESSOR_H_

#include <vector>
#include <string>
#include "CNNPostProcessor/CNNPostProcessor.h"

namespace xstream {
namespace CnnProc {
class AntiSpfPostProcessor : public CNNPostProcessor {
 public:
  AntiSpfPostProcessor() {}
  virtual ~AntiSpfPostProcessor() {}
  int Init(const std::string &cfg_path) override;
  int UpdateParameter(xstream::InputParamPtr ptr);
  std::vector<std::vector<BaseDataPtr> >
  DoProcess(const std::vector<std::vector<BaseDataPtr> > &input,
            const std::vector<xstream::InputParamPtr> &param) override;

 private:
  float anti_spf_threshold_ = 0.0f;
  BaseDataPtr
    FaceAntiSpfPostPro(const std::vector<std::vector<int8_t>> &mxnet_out,
                       int channel_size);
};
}  // namespace CnnProc
}  // namespace xstream
#endif  // INCLUDE_CNNPOSTPROCESSOR_ANTISPFPOSTPROCESSOR_H_
