/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.23
 */

#ifndef VEHICLE_SNAP_STRATEGY_CONFIG_PARAMS_TYPE_HPP_
#define VEHICLE_SNAP_STRATEGY_CONFIG_PARAMS_TYPE_HPP_

#include <memory>
#include <utility>
#include <unordered_map>
#include "vehicle_common_utility/data_type.hpp"
#include "vehicle_common_utility/utils.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

class FactorScoreParam {
 public:
  float weight = 1;
  NormFunType norm_fun = NormFunType::Linear;
  float min_value = 0.f;
  float max_value = 1.f;
  float norm_min = -5.f;
  float norm_max = 5.f;
};

struct EnumClassHash {
  template <typename T>
  std::size_t operator()(T t) const {
    return static_cast<std::size_t>(t);
  }
};

class SizeScoreParam : public FactorScoreParam {
 public:
  int valid_min_w = 0;
  int valid_min_h = 0;
  std::unordered_map<SizeCatgory, float, EnumClassHash> size_max;

  bool CheckIfSizeValid(const BBoxPtr &ptr_bbox) {
    float area = BBoxArea(*ptr_bbox);
    float valid_area = static_cast<float>(valid_min_w * valid_min_h);
    if (area > valid_area)
      return true;
    else
      return false;
  }
};

class IntegrityScoreParam : public FactorScoreParam {
 public:
  int dis_th = 0;
};

struct ScoreSelectiveSnapParam {
  float ignore_overlap_ratio = 0.8;  // Object in ignore area, ignore th
  int post_frame_threshold =
      800;                    // Ouput snap result, when object appear $1 frames
  int min_tracklet_len = 25;  // Above $1, a tracklet is valid
  float snap_min_score = 0.27;  // Below $1, do not post snap results
  float growth_ladder = 0.1;
};

struct CrossLineSnapParam {
  int line_width = 5;  // in pixels
  std::pair<PointInt, PointInt> line;
};

struct SnapControlParam {
  SnapMode snap_mode = SnapMode::ScoreSelectiveSnap;
  ScoreSelectiveSnapParam score_selective_snap_params;
  CrossLineSnapParam cross_line_snap_params;
};

typedef std::shared_ptr<FactorScoreParam> FactorScoreParamPtr;
typedef std::shared_ptr<SizeScoreParam> SizeScoreParamPtr;
typedef std::shared_ptr<IntegrityScoreParam> IntegrityScoreParamPtr;

struct WholeScoreParam {
  SizeScoreParamPtr vehicle_size_params = nullptr;
  SizeScoreParamPtr plate_size_params = nullptr;
  FactorScoreParamPtr vehicle_det_params = nullptr;
  FactorScoreParamPtr plate_det_params = nullptr;
  FactorScoreParamPtr visibility_params = nullptr;
  IntegrityScoreParamPtr integrity_params = nullptr;
};

typedef std::shared_ptr<WholeScoreParam> WholeScoreParamPtr;

VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // VEHICLE_SNAP_STRATEGY_CONFIG_PARAMS_TYPE_HPP_
