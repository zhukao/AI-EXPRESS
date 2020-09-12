/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.11.01
 */

#ifndef VEHICLE_SNAP_STRATEGY_SNAP_CONTROLLER_HPP_
#define VEHICLE_SNAP_STRATEGY_SNAP_CONTROLLER_HPP_

#include <memory>
#include "vehicle_common_utility/data_type.hpp"
#include "vehicle_snap_strategy/vehicle_snap_strategy_api.hpp"
#include "horizon/vision_type/vision_type.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

/*
 * @brief Control the snap output strategy.
 */
class SnapController {
 public:
  SnapController() = default;
  virtual ~SnapController() = default;

  /*
   * @brief Add a newly occurred target to the controller.
   * @param The target.
   */
  virtual bool AddTarget(const ImageFramePtr &frame, Vehicle &target) {
    return false;
  }

  /*
   * @brief Tell that the controller is going to be deleted. Must be called
   * before ``GetSnapResults``.
   */
  virtual void MarkToDelete() = 0;

  /*
   * @brief Get snap result.
   * @param & return If the return value is `true`, the snap result is
   * successfully returned and is contained in the `target`.
   * Else the snap result is not valid.
   */
  virtual bool GetSnapResults(Vehicle &target) { return false; }

 protected:
  /*
   * @brief Crop the image, must be implemented external
   */
  CropImage crop_image_;
};

typedef std::shared_ptr<SnapController> SnapControllerPtr;

VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // VEHICLE_SNAP_STRATEGY_SNAP_CONTROLLER_HPP_
