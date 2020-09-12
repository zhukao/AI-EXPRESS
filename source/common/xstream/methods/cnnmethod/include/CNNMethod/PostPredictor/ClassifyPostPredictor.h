/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: ClassifyPostPredictor.h
 * @Brief: declaration of the ClassifyPostPredictor
 * @Author: yatao.fu
 * @Email: yatao.fu@horizon.ai
 * @Date: 2019-09-5 16:18:28
 * @Last Modified by: yatao.fu
 * @Last Modified time: 16:18:28
 */

#ifndef CNNMETHOD_CASSIFYPOSTPREDICTOR_H
#define CNNMETHOD_CASSIFYPOSTPREDICTOR_H

#include <memory>
#include <vector>
#include "CNNMethod/PostPredictor/PostPredictor.h"

namespace xstream {
class ClassifyPostPredictor : public PostPredictor {
 public:
  virtual void Do(CNNMethodRunData *run_data);

 private:
  std::vector<int> TargetPro(
      const std::vector<std::vector<int8_t>> &mxnet_outs);

  std::vector<int> DefaultVaule(int size);
};
}  // namespace xstream

#endif  // CNNMETHOD_CASSIFYPOSTPREDICTOR_H
