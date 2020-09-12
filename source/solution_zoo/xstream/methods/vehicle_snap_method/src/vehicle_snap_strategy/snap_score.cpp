/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.25
 */

#include <memory>
#include "hobotlog/hobotlog.hpp"
#include "vehicle_snap_strategy/size_score.hpp"
#include "vehicle_snap_strategy/det_conf_score.hpp"
#include "vehicle_snap_strategy/visibility_score.hpp"
#include "vehicle_snap_strategy/integrity_score.hpp"
#include "vehicle_snap_strategy/config_params_type.hpp"
#include "vehicle_snap_strategy/snap_score.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

SnapScore::SnapScore(const int &img_width, const int &img_height,
                     const WholeScoreParam &snap_score_params) {
  Init(img_width, img_height, snap_score_params);
}

void SnapScore::Init(const int &img_width, const int &img_height,
                     const WholeScoreParam &snap_score_params) {
  if (snap_score_params.vehicle_size_params != nullptr) {
    PtrFactorScore vehicle_size_calculator =
        std::make_shared<SizeScore>(img_width, img_height, ObjectType::VEHICLE,
                                    snap_score_params.vehicle_size_params);
    factor_score_calculators_.push_back(vehicle_size_calculator);
    factor_score_params_.push_back(snap_score_params.vehicle_size_params);
  }

  if (snap_score_params.plate_size_params != nullptr) {
    PtrFactorScore plate_size_calculator =
        std::make_shared<SizeScore>(img_width, img_height, ObjectType::PLATE,
                                    snap_score_params.plate_size_params);
    factor_score_calculators_.push_back(plate_size_calculator);
    factor_score_params_.push_back(snap_score_params.plate_size_params);
  }

  if (snap_score_params.vehicle_det_params != nullptr) {
    PtrFactorScore vehicle_det_score_calculator =
        std::make_shared<DetConfScore>(ObjectType::VEHICLE,
                                       snap_score_params.vehicle_det_params);
    factor_score_calculators_.push_back(vehicle_det_score_calculator);
    factor_score_params_.push_back(snap_score_params.vehicle_det_params);
  }

  if (snap_score_params.plate_det_params != nullptr) {
    PtrFactorScore plate_det_score_calculator = std::make_shared<DetConfScore>(
        ObjectType::PLATE, snap_score_params.plate_det_params);
    factor_score_calculators_.push_back(plate_det_score_calculator);
    factor_score_params_.push_back(snap_score_params.plate_det_params);
  }

  if (snap_score_params.visibility_params != nullptr) {
    PtrFactorScore visibility_score_calculator =
        std::make_shared<VisibilityScore>(snap_score_params.visibility_params);
    factor_score_calculators_.push_back(visibility_score_calculator);
    factor_score_params_.push_back(snap_score_params.visibility_params);
  }

  if (snap_score_params.integrity_params != nullptr) {
    PtrFactorScore integrity_score_calculator =
        std::make_shared<IntegrityScore>(img_width, img_height,
                                         snap_score_params.integrity_params);
    factor_score_calculators_.push_back(integrity_score_calculator);
    factor_score_params_.push_back(snap_score_params.integrity_params);
  }
}

float SnapScore::GetSnapScore(const Vehicle &vehicle) const {
  return ComputeSnapScore(vehicle);
}

float SnapScore::ComputeSnapScore(const Vehicle &vehicle) const {
  float weight = 0.f;
  float score = 0.f;
  for (size_t i = 0; i < factor_score_calculators_.size(); ++i) {
    float tmp = factor_score_calculators_[i]->ComputeScore(vehicle);
    LOGD << "Track_ID " << vehicle.bbox->id << " snap_score " << i << " "
         << tmp;
    score += factor_score_params_[i]->weight * tmp;
    weight += factor_score_params_[i]->weight;
  }
  score /= weight;
  LOGD << "Track_ID " << vehicle.bbox->id << " final_score " << score;
  return score;
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
