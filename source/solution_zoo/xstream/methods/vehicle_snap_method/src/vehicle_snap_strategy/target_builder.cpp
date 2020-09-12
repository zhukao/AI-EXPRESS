/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.28
 */

#include <memory>
#include "hobotlog/hobotlog.hpp"
#include "vehicle_common_utility/utils.hpp"
#include "vehicle_snap_strategy/target_builder.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

TargetBuilder::TargetBuilder(const WholeScoreParam &snap_score_params,
                             const ScoreSelectiveSnapParam &snap_params) {
  snap_score_params_ = snap_score_params;
  snap_params_ = snap_params;
}

void TargetBuilder::BuildTarget(const ImageMetaType &image_meta_info,
                                VehicleListPtr &target_list) {
  if (snap_score_calculator_ == nullptr) {
    snap_score_calculator_ = std::make_shared<SnapScore>(
        image_meta_info.img_width, image_meta_info.img_height,
        snap_score_params_);
  }
  for (auto &iter : *target_list) {
    bool is_valid = CheckIfValid(iter.bbox);
    if (is_valid) {
      if (iter.plate != nullptr) {
        if (!snap_score_params_.plate_size_params->CheckIfSizeValid(
                 iter.plate->bbox))
          iter.plate->bbox->score = 0.f;
      }
      iter.occ_ratio = ComputeOccRatio(target_list, iter);
      iter.snap_score = snap_score_calculator_->GetSnapScore(iter);
      if (iter.snap_score < snap_params_.snap_min_score) is_valid = false;
    }
    iter.is_valid = is_valid;
    iter.time_stamp = image_meta_info.time_stamp;
  }
}

float TargetBuilder::ComputeOccRatio(const VehicleListPtr &target_list,
                                     const Vehicle &target) {
  float occ_area = 0.f;
  float target_area = BBoxArea(*target.bbox);
  for (const auto &iter : *target_list) {
    float insec_area = IntersectionArea(*iter.bbox, *target.bbox);
    if (iter.bbox->y2 > target.bbox->y2) occ_area += insec_area;
  }
  float occ_ratio = occ_area / target_area;
  occ_ratio = occ_ratio > 1.f ? 1.f : occ_ratio;
  return occ_ratio;
}

bool TargetBuilder::CheckIfValid(const BBoxPtr &ptr_bbox) {
  if (!CheckIfWhitinBlackArea(ptr_bbox) &&
      snap_score_params_.vehicle_size_params->CheckIfSizeValid(ptr_bbox))
    return true;
  else
    return false;
}

bool TargetBuilder::CheckIfWhitinBlackArea(const BBoxPtr &ptr_bbox) {
  float max_in_ratio = 0.f;
  for (const auto &iter : black_area_) {
    float in_ratio = BBoxInRatio(*ptr_bbox, iter);
    max_in_ratio = in_ratio > max_in_ratio ? in_ratio : max_in_ratio;
  }
  if (max_in_ratio > snap_params_.ignore_overlap_ratio)
    return true;
  else
    return false;
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
