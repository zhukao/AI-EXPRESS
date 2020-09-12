/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.11.02
 */

#include "gtest/gtest.h"
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "utils_test.hpp"
#include "vehicle_common_utility/data_type.hpp"
#include "vehicle_snap_strategy/config_params_type.hpp"
#include "vehicle_snap_strategy/cross_line_snap_controller.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

TEST(VehicleCrossLineStrategy, CrossLine) {
  CrossLineSnapParam params;
  params.line_width = 5;
  params.line =
      std::pair<PointInt, PointInt>{PointInt(0, 100), PointInt(1000, 100)};

  auto img = cv::imread("./config/data/donpolu_000001.jpg");
  auto img_frame = std::make_shared<hobot::vision::CVImageFrame>();
  img_frame->img = img;

  Vehicle target;
  target.lane_id = 0;
  target.bbox = std::make_shared<vision::BBox>(20, 20, 40, 70);
  target.bbox->id = 1;
  CrossLineSnapController snap_controller(crop_image, params, target);

  target.bbox = std::make_shared<vision::BBox>(20, 30, 40, 80);
  target.bbox->id = 1;
  bool return_val = snap_controller.AddTarget(img_frame, target);
  EXPECT_EQ(return_val, false);

  target.bbox = std::make_shared<vision::BBox>(20, 40, 40, 90);
  target.bbox->id = 1;
  return_val = snap_controller.AddTarget(img_frame, target);
  EXPECT_EQ(return_val, false);

  target.bbox = std::make_shared<vision::BBox>(20, 50, 40, 100);
  target.bbox->id = 1;
  return_val = snap_controller.AddTarget(img_frame, target);
  EXPECT_EQ(return_val, false);

  target.bbox = std::make_shared<vision::BBox>(20, 55, 40, 105);
  target.bbox->id = 1;
  return_val = snap_controller.AddTarget(img_frame, target);
  EXPECT_EQ(return_val, false);

  target.bbox = std::make_shared<vision::BBox>(20, 60, 40, 110);
  target.bbox->id = 1;
  return_val = snap_controller.AddTarget(img_frame, target);
  EXPECT_EQ(return_val, true);
  EXPECT_NE(target.sub_img, nullptr);
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
