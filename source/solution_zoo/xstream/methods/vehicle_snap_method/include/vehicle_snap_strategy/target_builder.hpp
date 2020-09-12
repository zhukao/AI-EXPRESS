/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.28
 */

#ifndef VEHICLE_SNAP_STRATEGY_TARGET_BUILDER_HPP_
#define VEHICLE_SNAP_STRATEGY_TARGET_BUILDER_HPP_

#include <vector>
#include <memory>
#include "vehicle_common_utility/data_type.hpp"
#include "vehicle_snap_strategy/snap_score.hpp"
#include "vehicle_snap_strategy/config_params_type.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

/*
 * @brief The class build the target, i.e., precessing the
 * input target list, e.g. compute the snap score.
 */
class TargetBuilder {
 public:
  /*
   * @brief Constructor.
   * @param image_meta_info, image meta information.
   * @param snap_score_params, params related to computing specific
   * snap scores.
   * @param snap_params, snap post logic control related params.
   */
  TargetBuilder(const WholeScoreParam &snap_score_params,
                const ScoreSelectiveSnapParam &snap_params);
  ~TargetBuilder() = default;

  /*
   * @brief Build the target, e.g., compute snap scores.
   * @param target_list, the list of targets.
   * @return the results will be wrote into `target_list`.
   */
  void BuildTarget(const ImageMetaType &image_meta_info,
                   VehicleListPtr &target_list);

  /*
   * @brief Set the black area.
   * @param black_area
   */
  void SetBlackArea(const std::vector<vision::BBox> &black_area) {
    black_area_ = black_area;
  }

 private:
  /*
   * @brief Compute occlusion ratiio.
   * @param target_list, total target in the frame.
   * @param target, the specific target.
   * @return occlusion ratiio.
   */
  float ComputeOccRatio(const VehicleListPtr &target_list,
                        const Vehicle &target);

  /*
   * @brief Check if target is valid.
   */
  bool CheckIfValid(const BBoxPtr &ptr_bbox);

  /*
   * @brief Check if the target whitin the black area.
   */
  bool CheckIfWhitinBlackArea(const BBoxPtr &ptr_bbox);

 private:
  WholeScoreParam snap_score_params_;
  ScoreSelectiveSnapParam snap_params_;

  std::vector<vision::BBox> black_area_;
  SnapScorePtr snap_score_calculator_ = nullptr;
};

typedef std::shared_ptr<TargetBuilder> PtrTargetBuilder;

VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // VEHICLE_SNAP_STRATEGY_TARGET_BUILDER_HPP_
