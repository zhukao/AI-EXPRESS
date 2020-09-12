/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  shiyu.fu
 * @version 0.0.1
 * @date  2020.04.14
 */

#include "gtest/gtest.h"
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "utils_test.hpp"
#include "vehicle_common_utility/data_type.hpp"
#include "vehicle_snap_strategy/config_params_type.hpp"
#include "vehicle_snap_strategy/score_selective_snap_controller.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

TEST(VehicleScoreSelectiveStrategy, ScoreSelective) {
  ScoreSelectiveSnapParam params;
  params.post_frame_threshold = 2;

  auto img = cv::imread("./config/data/donpolu_000001.jpg");
  auto img_frame = std::make_shared<hobot::vision::CVImageFrame>();
  img_frame->img = img;

  Vehicle target;
  target.lane_id = 0;
  target.bbox = std::make_shared<vision::BBox>(20, 20, 40, 70);
  target.bbox->id = 1;
  target.is_valid = true;
  target.snap_score = 0.6;
  ScoreSelectiveSnapController
      snap_controller(img_frame, crop_image, params, target);

  target.bbox = std::make_shared<vision::BBox>(20, 30, 40, 80);
  bool return_val = snap_controller.AddTarget(img_frame, target);
  EXPECT_EQ(return_val, false);
  bool return_val2 = snap_controller.GetSnapResults(target);
  EXPECT_EQ(return_val2, false);

  target.bbox = std::make_shared<vision::BBox>(20, 40, 40, 90);
  target.snap_score = 0.8;
  return_val = snap_controller.AddTarget(img_frame, target);
  EXPECT_EQ(return_val, false);
  return_val2 = snap_controller.GetSnapResults(target);
  EXPECT_EQ(return_val2, true);
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
