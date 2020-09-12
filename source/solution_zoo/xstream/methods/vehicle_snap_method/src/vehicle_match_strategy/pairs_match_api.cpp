/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.16
 */

#include "vehicle_match_strategy/pairs_match_api.hpp"
#include "vehicle_match_strategy/pairs_match.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

std::shared_ptr<PairsMatchAPI> PairsMatchAPI::NewPairsMatchAPI(
    const std::string &config_file) {
  return std::make_shared<PairsMatch>(config_file);
}

std::shared_ptr<PairsMatchAPI> PairsMatchAPI::NewPairsMatchAPI(
    const Params &params) {
  return std::make_shared<PairsMatch>(params);
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
