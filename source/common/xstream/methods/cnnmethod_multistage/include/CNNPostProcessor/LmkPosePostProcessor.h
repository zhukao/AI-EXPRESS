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

#ifndef INCLUDE_CNNPOSTPROCESSOR_LMKPOSEPOSTPROCESSOR_H_
#define INCLUDE_CNNPOSTPROCESSOR_LMKPOSEPOSTPROCESSOR_H_

#include <vector>
#include "CNNPostProcessor/CNNPostProcessor.h"

namespace xstream {
namespace CnnProc {
class LmkPosePostProcessor : public CNNPostProcessor {
 public:
  LmkPosePostProcessor() {}
  virtual ~LmkPosePostProcessor() {}
  std::vector<std::vector<BaseDataPtr> >
  DoProcess(const std::vector<std::vector<BaseDataPtr> > &input,
            const std::vector<xstream::InputParamPtr> &param) override;

 private:
  void HandleLmkPose(const std::vector<std::vector<int8_t>> &mxnet_outs,
                     const hobot::vision::BBox &box,
                     const std::vector<std::vector<uint32_t>> &nhwc,
                     std::vector<BaseDataPtr> *output);

  BaseDataPtr LmkPostPro(const std::vector<std::vector<int8_t>> &mxnet_outs,
                         const hobot::vision::BBox &box,
                         const std::vector<std::vector<uint32_t>> &nhwc);

  BaseDataPtr PosePostPro(const std::vector<int8_t> &mxnet_out);
};
}  // namespace CnnProc
}  // namespace xstream
#endif  // INCLUDE_CNNPOSTPROCESSOR_LMKPOSEPOSTPROCESSOR_H_
