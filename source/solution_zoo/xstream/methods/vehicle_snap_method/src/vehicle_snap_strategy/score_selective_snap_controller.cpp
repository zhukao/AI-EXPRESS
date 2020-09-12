/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.28
 */

#include "hobotlog/hobotlog.hpp"
#include "vehicle_snap_strategy/score_selective_snap_controller.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

ScoreSelectiveSnapController::ScoreSelectiveSnapController(
    const ImageFramePtr &frame, const CropImage &crop_image,
    const ScoreSelectiveSnapParam &snap_params, Vehicle &target) {
  params_ = snap_params;
  crop_image_ = crop_image;
  AddTarget(frame, target);
}

bool ScoreSelectiveSnapController::AddTarget(const ImageFramePtr &frame,
                                             Vehicle &target) {
  if (target.is_valid && !is_posted_) {
    ++select_count_;
    if (target.snap_score > max_snap_score_ + params_.growth_ladder) {
      max_snap_score_ = target.snap_score;
      target_ = target;
      auto bbox_w = target_.bbox->Width();
      auto bbox_h = target_.bbox->Height();
      target_.sub_img =
          crop_image_(frame, *target_.bbox, bbox_w, bbox_h, false);
    } else {
      target_.color = target.color;
      target_.model_type = target.model_type;
    }
  }

  if (!is_posted_ && select_count_ > params_.post_frame_threshold) {
    is_ready_to_post_ = true;
  }
  return false;
}

bool ScoreSelectiveSnapController::GetSnapResults(Vehicle &target) {
  if (!is_posted_ &&
      (is_ready_to_post_ ||
       (is_to_delete_ && select_count_ > params_.min_tracklet_len))) {
    is_posted_ = true;
    target = target_;
    return true;
  } else {
    return false;
  }
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
