/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: VehicleImgInputPredictor.h
 * @Brief: declaration of the VehicleImgInputPredictor
 * @Author: ronghui.zhang
 * @Email: ronghui.zhang@horizon.ai
 * @Date: 2020-03-02 14:52:31
 */

#ifndef INCLUDE_CNNMETHOD_PREDICTOR_VEHICLEIMGINPUTPREDICTOR_H_
#define INCLUDE_CNNMETHOD_PREDICTOR_VEHICLEIMGINPUTPREDICTOR_H_

#include <memory>
#include <vector>
#include "CNNMethod/Predictor/Predictor.h"
#include "CNNMethod/util/CNNMethodData.h"
#include "CNNMethod/Predictor/ImgInputPredictor.h"

namespace xstream {

class VehicleImgInputPredictor : public ImgInputPredictor {
 public:
    std::shared_ptr<BaseDataVector> RoisConvert(
        std::vector<BaseDataPtr> input) override;
 private:
    std::shared_ptr<BaseDataVector> PlateBoxProcess(
      std::shared_ptr<BaseDataVector> box_input,
      std::shared_ptr<BaseDataVector> plate_types);
};
}  // namespace xstream
#endif
