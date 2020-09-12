/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @Author: xudong.du
 * @Email: xudong.du@horizon.ai
 * @Date: 2020-05-16 14:18:28
 * @Last Modified by: xudong.du
 * @Last Modified time: 2020-06-06 15:18:28
 */
#ifndef INCLUDE_BACK_BONE_H_
#define INCLUDE_BACK_BONE_H_

#include <vector>
#include "CNNMethod/PostPredictor/PostPredictor.h"

namespace xstream {

class BackBonePostPredictor : public PostPredictor {
 public:
  virtual void Do(CNNMethodRunData *run_data);
  void HandleBackboneInfo(
      const std::vector<std::vector<int8_t>> &mxnet_output,
      std::vector<BaseDataPtr> *output,
      CNNMethodRunData *run_data,
      int index);
};
}  // namespace xstream
#endif  // INCLUDE_BACK_BONE_H_
