/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.11.01
 */

#include "hobotlog/hobotlog.hpp"
#include "vehicle_common_utility/utils.hpp"
#include "vehicle_snap_strategy/cross_line_snap_controller.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

CrossLineSnapController::CrossLineSnapController(
    const CropImage &crop_image, const CrossLineSnapParam &params,
    Vehicle &target) {
  crop_image_ = crop_image;
  params_ = params;

  status_ = ComputeStatus(target);
  ++count_;
}

bool CrossLineSnapController::AddTarget(const ImageFramePtr &frame,
                                        Vehicle &target) {
  auto current_status = ComputeStatus(target);
  bool res = false;
  if (!is_posted_ && CheckIfToPost(current_status)) {
    auto bbox_w = target.bbox->Width();
    auto bbox_h = target.bbox->Height();
    target.time_stamp = frame->time_stamp;
    target.sub_img = crop_image_(frame, *target.bbox, bbox_w, bbox_h, false);
    is_posted_ = true;
    res = true;
  }
  ++count_;
  return res;
}

bool CrossLineSnapController::CheckIfToPost(
    VehicleStatusWithLine &current_status) {
  bool res = false;
  if (count_ == 1 && status_ == VehicleStatusWithLine::OnSide) {
    res = true;
  } else {
    switch (current_status) {
      case VehicleStatusWithLine::UpSide:
        if (status_ == VehicleStatusWithLine::DownSide)
          res = true;
        else
          res = false;
        status_ = current_status;
        break;
      case VehicleStatusWithLine::DownSide:
        if (status_ == VehicleStatusWithLine::UpSide)
          res = true;
        else
          res = false;
        status_ = current_status;
        break;
      case VehicleStatusWithLine::OnSide:
      default:
        res = false;
        break;
    }
  }
  return res;
}

VehicleStatusWithLine CrossLineSnapController::ComputeStatus(Vehicle &target) {
  VehicleStatusWithLine current_status;
  PointInt t_point((target.bbox->x1 + target.bbox->x2) / 2, target.bbox->y2);
  if (t_point.x >= params_.line.first.x && t_point.x <= params_.line.second.x) {
    auto &line = params_.line;
    auto dis_to_line = DistanceToLine(t_point, line);
    int dis_x = line.first.x - line.second.x;
    int dis_y = line.second.y - line.first.y;
    if (std::abs(dis_to_line) > params_.line_width) {
      if (dis_x == 0) {
        if (dis_y * dis_to_line < 0)
          current_status = VehicleStatusWithLine::UpSide;
        else
          current_status = VehicleStatusWithLine::DownSide;
      } else {
        if (dis_x * dis_to_line < 0)
          current_status = VehicleStatusWithLine::UpSide;
        else
          current_status = VehicleStatusWithLine::DownSide;
      }
    } else {
      current_status = VehicleStatusWithLine::OnSide;
    }
  } else {
    current_status = VehicleStatusWithLine::Unknown;
  }
  return current_status;
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
