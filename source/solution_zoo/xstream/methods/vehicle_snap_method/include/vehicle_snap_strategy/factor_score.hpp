/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.24
 */

#ifndef VEHICLE_SNAP_STRATEGY_FACTOR_SCORE_HPP_
#define VEHICLE_SNAP_STRATEGY_FACTOR_SCORE_HPP_

#include <memory>
#include "vehicle_common_utility/data_type.hpp"
#include "vehicle_snap_strategy_namespace.hpp"
#include "vehicle_snap_strategy/config_params_type.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

class FactorScore {
 public:
  FactorScore() = default;
  virtual ~FactorScore() = default;

  /*
   * @brief Compute the score specificed by the snap factor.
   * Must be realized in child class.
   * @params vehicle, the target to compute snap score.
   * @return The snap score.
   */
  virtual float ComputeScore(const Vehicle &vehicle) = 0;

 protected:
  /*
   * @brief Norm the raw_score to [min_value, max_value], and
   * then map it with the specificed function to another space.
   * @param raw_score, which must be normalized to [0,1].
   * @return the re-normed and re-maped score.
   */
  float NormAndMap(float raw_score);

  FactorScoreParamPtr params_ = nullptr;
};

typedef std::shared_ptr<FactorScore> PtrFactorScore;

VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // VEHICLE_SNAP_STRATEGY_FACTOR_SCORE_HPP_
