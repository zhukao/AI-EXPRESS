/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: ImgInputPredictor.h
 * @Brief: declaration of the ImgInputPredictor
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-07-16 14:52:31
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-07-16 16:22:44
 */

#ifndef INCLUDE_CNNMETHOD_PREDICTOR_IMGINPUTPREDICTOR_H_
#define INCLUDE_CNNMETHOD_PREDICTOR_IMGINPUTPREDICTOR_H_

#include <memory>
#include <vector>
#include "CNNMethod/Predictor/Predictor.h"
#include "CNNMethod/util/CNNMethodData.h"
#include "horizon/vision_type/vision_type.hpp"
#include "horizon/vision_type/vision_type_common.h"

using hobot::vision::BBox;

namespace xstream {

class ImgInputPredictor : public Predictor {
 public:
  virtual int32_t Init(std::shared_ptr<CNNMethodConfig> config);
  virtual void Do(CNNMethodRunData *run_data);
  virtual void UpdateParam(std::shared_ptr<CNNMethodConfig> config);
  void DoPlateNum(CNNMethodRunData *run_data);

  virtual std::shared_ptr<BaseDataVector> RoisConvert(
      std::vector<BaseDataPtr> input);
 private:
  FilterMethod filter_method_ = FilterMethod::NO_FILTER;
  int rotate_degree_ = 0;
  NormParams norm_params_;
};
}  // namespace xstream
#endif  // INCLUDE_CNNMETHOD_PREDICTOR_IMGINPUTPREDICTOR_H_
