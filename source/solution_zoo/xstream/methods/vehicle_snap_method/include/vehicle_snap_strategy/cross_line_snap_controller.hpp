/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.11.01
 */

#ifndef VEHICLE_SNAP_STRATEGY_CROSS_LINE_SNAP_CONTROLLER_HPP_
#define VEHICLE_SNAP_STRATEGY_CROSS_LINE_SNAP_CONTROLLER_HPP_

#include "vehicle_snap_strategy/snap_controller.hpp"
#include "vehicle_snap_strategy/config_params_type.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

enum class VehicleStatusWithLine {
  Unknown = 0,
  UpSide = 1,
  OnSide = 2,
  DownSide = 3
};

/*
 * @brief Buffer the status of targets and
 * output the one which just cross line.
 */
class CrossLineSnapController : public SnapController {
 public:
  CrossLineSnapController() = default;
  ~CrossLineSnapController() = default;

  CrossLineSnapController(const CropImage &crop_image,
                          const CrossLineSnapParam &params, Vehicle &target);

  /*
   * @brief Add a newly occurred target to the controller.
   * @param The target.
   * @return, boolean, if is true, the returned target is stored in param
   * target, else, the returned target is not valid.
   */
  bool AddTarget(const ImageFramePtr &frame, Vehicle &target) override;

  /*
   * @brief Tell that the controller is going to be deleted. Must be called
   * before ``GetSnapResults``.
   */
  void MarkToDelete() override {}

 private:
  /*
   * @brief Compute the status of vehicle withon the abrtract cross line
   * @param target, the vehicle.
   * @return the status.
   */
  VehicleStatusWithLine ComputeStatus(Vehicle &target);

  /*
   * @biref Check if it is ready to post snap.
   * @params current_status
   * @return boolean
   */
  bool CheckIfToPost(VehicleStatusWithLine &current_status);

 private:
  CrossLineSnapParam params_;

  bool is_posted_ = false;
  VehicleStatusWithLine status_ = VehicleStatusWithLine::Unknown;
  int count_ = 0;
};

VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // VEHICLE_SNAP_STRATEGY_CROSS_LINE_SNAP_CONTROLLER_HPP_
