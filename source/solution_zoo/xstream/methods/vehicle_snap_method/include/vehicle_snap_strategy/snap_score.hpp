/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.25
 */

#ifndef VEHICLE_SNAP_STRATEGY_SNAP_SCORE_HPP_
#define VEHICLE_SNAP_STRATEGY_SNAP_SCORE_HPP_

#include <vector>
#include <memory>
#include "vehicle_snap_strategy/factor_score.hpp"
#include "vehicle_common_utility/data_type.hpp"
#include "vehicle_snap_strategy/config_params_type.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

class SnapScore {
 public:
  SnapScore() = default;

  /*
   * @brief Constructor
   * @param img_width
   * @param img_height
   * @param snap_score_params, specific snap score related params.
   */
  SnapScore(const int &img_width, const int &img_height,
            const WholeScoreParam &snap_score_params);

  ~SnapScore() = default;

  /*
   * @brief Get the snap score.
   * @param vehicle, the target to compute snap score.
   * @return the total snap score.
   */
  float GetSnapScore(const Vehicle &vehicle) const;

 private:
  /*
   * @brief Init the class members according to specified configs.
   * @param img_width
   * @param img_height
   * @snap_score_params, specific snap score related params.
   */
  void Init(const int &img_width, const int &img_height,
            const WholeScoreParam &snap_score_params);

  /*
   * @brief Internal function that actually compute snap score.
   * @param vehicle, the target to compute snap score.
   * @return the total snap score.
   */
  float ComputeSnapScore(const Vehicle &vehicle) const;

 private:
  std::vector<FactorScoreParamPtr> factor_score_params_;
  std::vector<PtrFactorScore> factor_score_calculators_;
};

typedef std::shared_ptr<SnapScore> SnapScorePtr;

VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // VEHICLE_SNAP_STRATEGY_SNAP_SCORE_HPP_
