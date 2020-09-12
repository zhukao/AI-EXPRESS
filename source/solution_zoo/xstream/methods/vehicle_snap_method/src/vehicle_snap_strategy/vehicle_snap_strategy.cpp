/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.28
 */

#include <memory>
#include "hobotlog/hobotlog.hpp"
#include "vehicle_snap_strategy/score_selective_snap_controller.hpp"
#include "vehicle_snap_strategy/cross_line_snap_controller.hpp"
#include "vehicle_snap_strategy/parse_config_from_json.hpp"
#include "vehicle_snap_strategy/vehicle_snap_strategy.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

VehicleSnapStrategy::VehicleSnapStrategy(const std::string &config_file_name,
                                         const CropImage &crop_image) {
  parser_ptr_ = std::make_shared<ParseJsonSnap>(config_file_name);
  crop_image_ = crop_image;

  Init();
}

void VehicleSnapStrategy::Init() {
  snap_post_control_params_ = parser_ptr_->snap_post_control_params_;
  snap_score_params_ = parser_ptr_->snap_score_params_;
  ptr_target_builder_ = std::make_shared<TargetBuilder>(
      snap_score_params_,
      snap_post_control_params_.score_selective_snap_params);

  ptr_target_builder_->SetBlackArea(parser_ptr_->black_area_);
}

VehicleListPtr VehicleSnapStrategy::Process(
    const ImageFramePtr &frame, VehicleListPtr &target_list,
    const TrackIdListPtr &disappeared_track_ids) {
  VehicleListPtr results = nullptr;
  switch (snap_post_control_params_.snap_mode) {
    case SnapMode::ScoreSelectiveSnap:
      results = ScoreSelectiveControlledProcess(frame, target_list,
                                                disappeared_track_ids);
      break;
    case SnapMode::CrossLineSnap:
      results =
          CrossLineControlledProcess(frame, target_list, disappeared_track_ids);
      break;
    default:
      break;
  }
  return results;
}

Status VehicleSnapStrategy::SetSnapMode(const SnapMode &snap_mode) {
  Status res;
  try {
    snap_post_control_params_.snap_mode = snap_mode;
    controllers_.clear();
  }
  catch (const std::exception &e) {
    LOGE << " a standard exception was caught, with message '" << e.what()
         << "'";
    res.value = StatusType::kInvalidParam;
    res.info = e.what();
  }
  return res;
}

Status VehicleSnapStrategy::SetLine(const std::pair<PointInt, PointInt> &line) {
  Status res;
  if (snap_post_control_params_.snap_mode == SnapMode::CrossLineSnap) {
    try {
      snap_post_control_params_.cross_line_snap_params.line = line;
      controllers_.clear();
    }
    catch (const std::exception &e) {
      LOGE << " a standard exception was caught, with message '" << e.what()
           << "'";
      res.value = StatusType::kInvalidParam;
      res.info = e.what();
    }
  }
  return res;
}

Status VehicleSnapStrategy::SetBlackArea(
    const std::vector<vision::BBox> &black_area) {
  Status res;
  try {
    ptr_target_builder_->SetBlackArea(black_area);
  }
  catch (const std::exception &e) {
    LOGE << " a standard exception was caught, with message '" << e.what()
         << "'";
    res.value = StatusType::kInvalidParam;
    res.info = e.what();
  }
  return res;
}

VehicleListPtr VehicleSnapStrategy::ScoreSelectiveControlledProcess(
    const ImageFramePtr &frame, VehicleListPtr &target_list,
    const TrackIdListPtr &disappeared_track_ids) {
  ImageMetaType image_meta_info;
  image_meta_info.img_width = frame->Width();
  image_meta_info.img_height = frame->Height();
  image_meta_info.time_stamp = frame->time_stamp;

  ptr_target_builder_->BuildTarget(image_meta_info, target_list);

  VehicleListPtr results = std::make_shared<VehicleList>();
  for (auto &iter : *target_list) {
    if (controllers_.find(iter.bbox->id) == controllers_.end()) {
      controllers_.insert(
          {iter.bbox->id,
           std::make_shared<ScoreSelectiveSnapController>(
               frame, crop_image_,
               snap_post_control_params_.score_selective_snap_params, iter)});
    } else {
      controllers_[iter.bbox->id]->AddTarget(frame, iter);
      Vehicle target_tmp;
      bool return_val = controllers_[iter.bbox->id]->GetSnapResults(target_tmp);
      if (return_val) results->push_back(target_tmp);
    }
  }

  for (auto &it : *disappeared_track_ids) {
    if (controllers_.find(it) == controllers_.end()) {
      LOGI << "Disappeared track id is not found, ignore!";
      continue;
    }
    controllers_[it]->MarkToDelete();
    Vehicle target_tmp;
    bool return_val = controllers_[it]->GetSnapResults(target_tmp);
    if (return_val) results->push_back(target_tmp);

    controllers_.erase(it);
  }

  return results;
}

VehicleListPtr VehicleSnapStrategy::CrossLineControlledProcess(
    const ImageFramePtr &frame, VehicleListPtr &target_list,
    const TrackIdListPtr &disappeared_track_ids) {
  VehicleListPtr results = std::make_shared<VehicleList>();
  for (auto &iter : *target_list) {
    if (controllers_.find(iter.bbox->id) == controllers_.end()) {
      controllers_.insert(
          {iter.bbox->id,
           std::make_shared<CrossLineSnapController>(
               crop_image_, snap_post_control_params_.cross_line_snap_params,
               iter)});
    } else {
      bool return_val = controllers_[iter.bbox->id]->AddTarget(frame, iter);
      if (return_val) results->push_back(iter);
    }
  }

  for (auto &it : *disappeared_track_ids) {
    if (controllers_.find(it) == controllers_.end()) {
      LOGI << "Disappeared track id is not found, ignore!";
      continue;
    }
    controllers_.erase(it);
  }
  return results;
}

Status VehicleSnapStrategy::UpdateSnapParams(const std::string &param_str) {
  Status res;
  try {
    parser_ptr_->ParseSnapParamsFromString(param_str);
    Init();
    controllers_.clear();
  }
  catch (const std::exception &e) {
    LOGE << " a standard exception was caught, with message '" << e.what()
         << "'";
    res.value = StatusType::kInvalidParam;
    res.info = e.what();
  }
  return res;
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
