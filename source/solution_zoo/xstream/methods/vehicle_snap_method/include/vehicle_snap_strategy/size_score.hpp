/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.25
 */

#ifndef VEHICLE_SNAP_STRATEGY_SIZE_SCORE_HPP_
#define VEHICLE_SNAP_STRATEGY_SIZE_SCORE_HPP_

#include "hobotlog/hobotlog.hpp"
#include "vehicle_common_utility/utils.hpp"
#include "vehicle_snap_strategy/factor_score.hpp"
#include "vehicle_common_utility/data_type.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

/*
 * @brief The class compute the snap score of target size
 */
class SizeScore : public FactorScore {
 public:
  /*
   * @brief Constructor.
   * @param img_width
   * @param img_height
   * @param object_type, the type of target, either PLATE of VEHICLE
   * @param params, the snap score of target size related param.
   */
  SizeScore(const int &img_width, const int &img_height,
            const ObjectType &object_type, const SizeScoreParamPtr &params)
      : img_width_(img_width),
        img_height_(img_height),
        specific_params_(params),
        object_type_(object_type)
         {
    params_ = params;
  }
  ~SizeScore() = default;

  /*
   * @brief Compute the snap score to the target size.
   * @param vehicle, the target to compute the snap score.
   * @return the snap score of target size.
   */
  float ComputeScore(const Vehicle &vehicle) override {
    float area;
    float area_max = 1.f;
    switch (object_type_) {
      case ObjectType::PLATE: {
        if (vehicle.plate != nullptr &&
            specific_params_->CheckIfSizeValid(vehicle.plate->bbox))
          area = BBoxArea(*(vehicle.plate->bbox));
        else
          area = 0.f;
        area_max = specific_params_->size_max[SizeCatgory::Unknown_Size];
        break;
      }
      case ObjectType::VEHICLE: {
        if (specific_params_->CheckIfSizeValid(vehicle.bbox))
          area = BBoxArea(*(vehicle.bbox));
        else
          area = 0.f;
        SizeCatgory size_catgory = GetVehicleSizeCatgory(
            static_cast<VehicleModelType>(vehicle.model_type));
        auto itr = specific_params_->size_max.find(size_catgory);
        if (itr != specific_params_->size_max.end())
          area_max = itr->second;
        else
          area_max = specific_params_->size_max[SizeCatgory::Unknown_Size];
        break;
      }
      default: {
        LOGI << "Object type is not recognized, return zero";
        area = 0.f;
        break;
      }
    }
    area /= area_max;
    return NormAndMap(area);
  }

 private:
  int img_width_ = 1920;
  int img_height_ = 1080;

  SizeScoreParamPtr specific_params_;
  ObjectType object_type_;
};

VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // VEHICLE_SNAP_STRATEGY_SIZE_SCORE_HPP_
