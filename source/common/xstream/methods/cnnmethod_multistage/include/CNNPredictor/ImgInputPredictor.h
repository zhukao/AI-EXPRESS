/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: PreProcessor.h
 * @Brief: declaration of the ImgInputPredictor.h
 * @Author: ronghui.zhang
 * @Date: 2020-01-20 11:26:50
 * @Last Modified by: ronghui.zhang
 * @Last Modified time: 2020-01-20 11:26:50
 */

#ifndef INCLUDE_PREDICTOR_RESIZERPREDICTOR_H_
#define INCLUDE_PREDICTOR_RESIZERPREDICTOR_H_

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "CNNPredictor/CNNPredictor.h"

namespace xstream {
namespace CnnProc {
class ImgInputPredictor : public CNNPredictor {
 public:
  ImgInputPredictor() {}
  virtual ~ImgInputPredictor() {}
  int Init(const std::string &cfg_path) override;
  std::vector<std::vector<BaseDataPtr>>
  DoProcess(const std::vector<std::vector<BaseDataPtr> > &input,
            const std::vector<xstream::InputParamPtr> &param) override;
  int UpdateParameter(xstream::InputParamPtr ptr) override;

  virtual std::shared_ptr<BaseDataVector> RoisConvert(
      std::vector<BaseDataPtr> input);
 private:
  float expand_scale_ = 0.0f;
  NormMethod norm_method_ = NormMethod::BPU_MODEL_NORM_BY_NOTHING;
  FilterMethod filter_method_ = FilterMethod::NO_FILTER;
  int rotate_degree_ = 0;
  NormParams norm_params_;
};
}  // namespace CnnProc
}  // namespace xstream
#endif  // INCLUDE_PREDICTOR_RESIZERPREDICTOR_H_
