/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.28
 */

#include "vehicle_snap_strategy/vehicle_snap_strategy.hpp"
#include "vehicle_snap_strategy/vehicle_snap_strategy_api.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

std::shared_ptr<VehicleSnapStrategyAPI>
VehicleSnapStrategyAPI::NewVehicleSnapStrategyAPI(
    const std::string &config_file_name, const CropImage &crop_image) {
  return std::make_shared<VehicleSnapStrategy>(config_file_name, crop_image);
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
