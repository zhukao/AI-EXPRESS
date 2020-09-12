/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.28
 */

#ifndef VEHICLE_SNAP_STRATEGY_VISIBILITY_SCORE_HPP_
#define VEHICLE_SNAP_STRATEGY_VISIBILITY_SCORE_HPP_

#include "vehicle_snap_strategy/config_params_type.hpp"
#include "vehicle_snap_strategy/factor_score.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

/*
 * @brief The class compute the snap score of vehicle visibility
 */
class VisibilityScore : public FactorScore {
 public:
  /*
   * @brief Constructor.
   * @param params, the snap score of vehicle visibility related param.
   */
  explicit VisibilityScore(FactorScoreParamPtr params) { params_ = params; }

  ~VisibilityScore() = default;

  /*
   * @brief Compute the snap score to the vehicle visibility.
   * @param vehicle, the target to compute the snap score.
   * @return the snap score of vehicle visibility.
   */
  float ComputeScore(const Vehicle &vehicle) override {
    float score = 1.f - vehicle.occ_ratio;
    return NormAndMap(score);
  }
};

VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // VEHICLE_SNAP_STRATEGY_VISIBILITY_SCORE_HPP_
