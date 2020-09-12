/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.28
 */

#ifndef VEHICLE_SNAP_STRATEGY_DET_CONF_SCORE_HPP_
#define VEHICLE_SNAP_STRATEGY_DET_CONF_SCORE_HPP_

#include "hobotlog/hobotlog.hpp"
#include "vehicle_snap_strategy/factor_score.hpp"
#include "vehicle_common_utility/data_type.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

/*
 * @brief The class compute the snap score of detection confidence
 */
class DetConfScore : public FactorScore {
 public:
  /*
   * @brief Constructor.
   * @param object_type, the type of target, either PLATE of VEHICLE
   * @param params, the snap score of detection confidence related param.
   */
  DetConfScore(const ObjectType &object_type, FactorScoreParamPtr params)
      : object_type_(object_type) {
    params_ = params;
  }

  ~DetConfScore() = default;

  /*
   * @brief Compute the snap score to the detection confidence.
   * @param vehicle, the target to compute the snap score.
   * @return the snap score of detection confidence.
   */
  float ComputeScore(const Vehicle &vehicle) override {
    float score;
    switch (object_type_) {
      case ObjectType::PLATE:
        if (vehicle.plate != nullptr)
          score = vehicle.plate->bbox->score;
        else
          score = 0.f;
        break;
      case ObjectType::VEHICLE:
        score = vehicle.bbox->score;
        break;
      default:
        LOGI << "Object type is not recognized, return zero";
        score = 0.f;
        break;
    }
    score -= params_->min_value;
    score /= (params_->max_value - params_->min_value);
    return NormAndMap(score);
  }

 private:
  ObjectType object_type_;
};

VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // VEHICLE_SNAP_STRATEGY_DET_CONF_SCORE_HPP_
