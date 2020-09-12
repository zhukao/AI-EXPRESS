/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.28
 */

#ifndef VEHICLE_SNAP_STRATEGY_SCORE_SELECTIVE_SNAP_CONTROLLER_HPP_
#define VEHICLE_SNAP_STRATEGY_SCORE_SELECTIVE_SNAP_CONTROLLER_HPP_

#include "vehicle_snap_strategy/snap_controller.hpp"
#include "vehicle_snap_strategy/vehicle_snap_strategy_api.hpp"
#include "horizon/vision_type/vision_type.hpp"
#include "vehicle_common_utility/data_type.hpp"
#include "vehicle_snap_strategy/config_params_type.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

/*
 * @brief Buffer the max snap score target and
 * output the one with max snap score.
 */
class ScoreSelectiveSnapController : public SnapController {
 public:
  ScoreSelectiveSnapController() = default;
  ~ScoreSelectiveSnapController() = default;

  /*
   * @brief Constructor
   * @param snap_params, the snap post control params.
   * @param crop_image, functional hander to crop image.
   * @param target, the target to compute be added into the tracklet.
   */
  ScoreSelectiveSnapController(const ImageFramePtr &frame,
                               const CropImage &crop_image,
                               const ScoreSelectiveSnapParam &params,
                               Vehicle &target);

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
  void MarkToDelete() override { is_to_delete_ = true; }

  /*
   * @brief Get snap result.
   * @param & return If the return value is `true`, the snap result is
   * successfully returned and is contained in the `target`.
   * Else the snap result is not valid.
   */
  bool GetSnapResults(Vehicle &target) override;

 private:
  ScoreSelectiveSnapParam params_;
  float max_snap_score_ = -1.f;
  int  select_count_ = 0;
  bool is_posted_ = false;
  bool is_ready_to_post_ = false;
  bool is_to_delete_ = false;
  Vehicle target_;
};

VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // VEHICLE_SNAP_STRATEGY_SCORE_SELECTIVE_SNAP_CONTROLLER_HPP_
