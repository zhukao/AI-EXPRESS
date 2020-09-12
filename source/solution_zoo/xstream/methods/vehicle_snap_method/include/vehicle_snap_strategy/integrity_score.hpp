/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.28
 */

#ifndef VEHICLE_SNAP_STRATEGY_INTEGRITY_SCORE_HPP_
#define VEHICLE_SNAP_STRATEGY_INTEGRITY_SCORE_HPP_

#include <algorithm>
#include "vehicle_snap_strategy/factor_score.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

/*
 * @brief The class compute the snap score of vehicle integrity
 */
class IntegrityScore : public FactorScore {
 public:
  /*
   * @brief Constructor.
   * @param img_width
   * @param img_height
   * @param params, the snap score of vehicle integrity related param.
   */
  IntegrityScore(int img_width, int img_height,
                 IntegrityScoreParamPtr params)
      : img_width_(img_width),
        img_height_(img_height),
        specific_params_(params) {
    params_ = params;
  }
  ~IntegrityScore() = default;

  /*
   * @brief Compute the snap score to the vehicle integrity.
   * @param vehicle, the target to compute the snap score.
   * @return the snap score of vehicle integrity.
   */
  float ComputeScore(const Vehicle &vehicle) override {
    auto &bbox = vehicle.bbox;
    float dis_x2_to_border = img_width_ - bbox->x2;
    float dis_y2_to_border = img_height_ - bbox->y2;
    float min_dis = std::min(dis_x2_to_border, dis_y2_to_border);
    min_dis = std::min(min_dis, bbox->x1);
    min_dis = std::min(min_dis, bbox->y1);

    float score = 1.f;
    if (min_dis < specific_params_->dis_th) score = 0.f;

    return NormAndMap(score);
  }

 private:
  int img_width_ = 1920;
  int img_height_ = 1080;

  IntegrityScoreParamPtr specific_params_ = nullptr;
};

VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // VEHICLE_SNAP_STRATEGY_INTEGRITY_SCORE_HPP_
