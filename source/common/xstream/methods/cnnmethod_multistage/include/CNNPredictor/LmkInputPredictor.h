/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: LmkInputPredictor.h
 * @Brief: declaration of the LmkInputPredictor
 * @Author: zhe.sun
 * @Date: 2020-01-20 11:26:50
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-01-20 11:26:50
 */

#ifndef INCLUDE_CNNPREDICTOR_LMKINPUTPREDICTOR_H_
#define INCLUDE_CNNPREDICTOR_LMKINPUTPREDICTOR_H_

#include <string>
#include <vector>
#include <memory>
#include "CNNPredictor/CNNPredictor.h"
#include "hobotxsdk/xstream_data.h"

namespace xstream {
namespace CnnProc {
class LmkInputPredictor : public CNNPredictor {
 public:
  LmkInputPredictor() {}
  virtual ~LmkInputPredictor() {}

  std::vector<std::vector<BaseDataPtr> >
  DoProcess(const std::vector<std::vector<BaseDataPtr> > &input,
            const std::vector<xstream::InputParamPtr> &param) override;
};
}  // namespace CnnProc
}  // namespace xstream
#endif  // INCLUDE_CNNPREDICTOR_LMKINPUTPREDICTOR_H_
