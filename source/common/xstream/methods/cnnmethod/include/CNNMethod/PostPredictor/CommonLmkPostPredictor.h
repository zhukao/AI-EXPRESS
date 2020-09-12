/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: CommonLmkPostPredictor.h
 * @Brief: declaration of the CommonLmkPostPredictor
 * @Author: fei.cheng
 * @Email: fei.cheng@horizon.ai
 * @Date: 2020-07-18 14:18:28
 * @Last Modified by: fei.cheng
 * @Last Modified time: 2019-07-18 15:13:07
 */

#ifndef INCLUDE_CNNMETHOD_POSTPREDICTOR_COMMONLMKPOSTPREDICTOR_H_
#define INCLUDE_CNNMETHOD_POSTPREDICTOR_COMMONLMKPOSTPREDICTOR_H_

#include <vector>
#include <memory>
#include "CNNMethod/PostPredictor/PostPredictor.h"

namespace xstream {

class CommonLmkPostPredictor : public PostPredictor {
 public:
  int32_t Init(std::shared_ptr<CNNMethodConfig> config) override;
  virtual void Do(CNNMethodRunData *run_data);

 private:
  void HandleLmk(const std::vector<std::vector<int8_t>> &mxnet_outs,
                     const hobot::vision::BBox &box,
                     const std::vector<std::vector<uint32_t>> &nhwc,
                     const std::vector<std::vector<uint32_t>> &shifts,
                     std::vector<BaseDataPtr> *output);

  BaseDataPtr Lmk3Post(const std::vector<std::vector<int8_t>> &mxnet_outs,
                       const hobot::vision::BBox &box,
                       const std::vector<std::vector<uint32_t>> &nhwc,
                       const std::vector<std::vector<uint32_t>> &shifts);

  int CalIndex(int k, int i, int j);
  size_t lmk_num_;
  int feature_w_;
  int feature_h_;
  int i_o_stride_;
};
}  // namespace xstream
#endif  // INCLUDE_CNNMETHOD_POSTPREDICTOR_HANDLMKPOSTPREDICTOR_H_
