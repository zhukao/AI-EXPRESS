/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.24
 */

#include "vehicle_snap_strategy/factor_score.hpp"
#include "vehicle_common_utility/utils.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

float FactorScore::NormAndMap(float raw_score) {
  raw_score *= (params_->norm_max - params_->norm_min);
  raw_score += params_->norm_min;
  return MapFunction(raw_score, params_->norm_fun, params_->norm_min,
                     params_->norm_max);
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
