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

#ifndef CNNMETHOD_CASSIFYPOSTPROCESSOR_H
#define CNNMETHOD_CASSIFYPOSTPROCESSOR_H

#include <memory>
#include <vector>
#include <string>
#include "CNNPostProcessor/CNNPostProcessor.h"

namespace xstream {
namespace CnnProc {
class ClassifyPostProcessor : public CNNPostProcessor {
 public:
  ClassifyPostProcessor() {}
  virtual ~ClassifyPostProcessor() {}
  std::vector<std::vector<BaseDataPtr> >
  DoProcess(const std::vector<std::vector<BaseDataPtr> > &input,
            const std::vector<xstream::InputParamPtr> &param) override;

 private:
  std::vector<int> TargetPro(
      const std::vector<std::vector<int8_t>> &mxnet_outs);

  std::vector<int> DefaultVaule(int size);
};
}  // namespace CnnProc
}  // namespace xstream
#endif  // CNNMETHOD_CASSIFYPOSTPROCESSOR_H
