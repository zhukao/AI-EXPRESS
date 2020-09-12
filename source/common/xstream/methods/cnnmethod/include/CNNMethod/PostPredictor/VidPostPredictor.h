/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @Author: xudong.du
 * @Email: xudong.du@horizon.ai
 * @Date: 2020-05-16 14:18:28
 * @Last Modified by: xudong.du
 * @Last Modified time: 2020-06-06 15:18:28
 */
#ifndef INCLUDE_VID_H_
#define INCLUDE_VID_H_

#include <vector>
#include "CNNMethod/PostPredictor/PostPredictor.h"

namespace xstream {

class VidPostPredictor : public PostPredictor {
 public:
  virtual void Do(CNNMethodRunData *run_data);
};
}  // namespace xstream
#endif  // INCLUDE_VID_H_
