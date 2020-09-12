/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: BinaryClassifyPostPredictor.h
 * @Brief: declaration of the BinaryClassifyPostPredictor
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-05-16 14:18:28
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-05-16 14:18:28
 */

#ifndef INCLUDE_CNNMETHOD_POSTPREDICTOR_BINARYCLASSIFYPOSTPREDICTOR_H_
#define INCLUDE_CNNMETHOD_POSTPREDICTOR_BINARYCLASSIFYPOSTPREDICTOR_H_

#include <vector>
#include "CNNMethod/PostPredictor/PostPredictor.h"

namespace xstream {

class BinaryClassifyPostPredictor : public PostPredictor {
 public:
  virtual void Do(CNNMethodRunData *run_data);

 private:
  void HandleResult(const std::vector<std::vector<int8_t>> &mxnet_outs,
                       std::vector<BaseDataPtr> *output);
  BaseDataPtr PostPro(const std::vector<int8_t> &mxnet_out);
};
}  // namespace xstream
#endif  // INCLUDE_CNNMETHOD_POSTPREDICTOR_BINARYCLASSIFYPOSTPREDICTOR_H_
