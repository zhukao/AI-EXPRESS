/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.16
 */

#ifndef VEHICLE_MATCH_STRATEGY_CONFIG_PARAMS_TYPE_HPP_
#define VEHICLE_MATCH_STRATEGY_CONFIG_PARAMS_TYPE_HPP_

#include <memory>
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

struct Params {
  int img_width = 1920;
  int img_height = 1080;
  int img_cells = 2;
  int bbox_cells = 4;
  float overlap_th = 0.90;
  int min_statis_frames = 500;
  int work_frames = 2500;
  float max_pair_dis = 8.f;
  int min_stat_value = 5;
};

typedef std::shared_ptr<Params> ParamsPtr;

VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // VEHICLE_MATCH_STRATEGY_CONFIG_PARAMS_TYPE_HPP_
