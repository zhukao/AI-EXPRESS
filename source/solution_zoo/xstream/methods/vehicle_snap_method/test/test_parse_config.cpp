/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.24
 */

#include <string>
#include "gtest/gtest.h"
#include "vehicle_common_utility/data_type.hpp"
#include "vehicle_snap_strategy/config_params_type.hpp"
#include "vehicle_snap_strategy_namespace.hpp"
#include "vehicle_snap_strategy/parse_config_from_json.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

TEST(ConfigParseModule, ParseJsonSnap) {
  // SetLogLevel(HOBOT_LOG_DEBUG);
  std::string config_file_name("./config/data/config_snap_for_unit_test.json");
  auto parser = ParseJsonSnap(config_file_name);

  EXPECT_EQ(parser.snap_post_control_params_.snap_mode,
            SnapMode::CrossLineSnap);
  EXPECT_FLOAT_EQ(parser.snap_post_control_params_.score_selective_snap_params
                      .ignore_overlap_ratio,
                  0.93);
  EXPECT_EQ(parser.snap_post_control_params_.score_selective_snap_params
                .post_frame_threshold,
            600);
  EXPECT_EQ(parser.snap_post_control_params_.score_selective_snap_params
                .min_tracklet_len,
            36);
  EXPECT_FLOAT_EQ(parser.snap_post_control_params_.score_selective_snap_params
                      .snap_min_score,
                  0.42);

  EXPECT_EQ(parser.snap_post_control_params_.cross_line_snap_params.line_width,
            7);
  EXPECT_EQ(
      parser.snap_post_control_params_.cross_line_snap_params.line.first.x, 1);
  EXPECT_EQ(
      parser.snap_post_control_params_.cross_line_snap_params.line.first.y, 2);
  EXPECT_EQ(
      parser.snap_post_control_params_.cross_line_snap_params.line.second.x, 3);
  EXPECT_EQ(
      parser.snap_post_control_params_.cross_line_snap_params.line.second.y, 4);

  EXPECT_FLOAT_EQ(parser.snap_score_params_.vehicle_size_params->norm_min, -10);
  EXPECT_FLOAT_EQ(parser.snap_score_params_.vehicle_size_params->norm_max, 10);

  EXPECT_FLOAT_EQ(parser.snap_score_params_.vehicle_size_params->weight, 1);
  EXPECT_EQ(parser.snap_score_params_.vehicle_size_params->norm_fun,
            NormFunType::Sigmoid);
  EXPECT_EQ(parser.snap_score_params_.vehicle_size_params->valid_min_w, 35);
  EXPECT_EQ(parser.snap_score_params_.vehicle_size_params->valid_min_h, 35);
  EXPECT_FLOAT_EQ(parser.snap_score_params_.vehicle_size_params->size_max
                      [SizeCatgory::Unknown_Size],
                  1e6);
  EXPECT_FLOAT_EQ(parser.snap_score_params_.vehicle_size_params->size_max
                      [SizeCatgory::Small],
                  1e5);
  EXPECT_FLOAT_EQ(parser.snap_score_params_.vehicle_size_params->size_max
                      [SizeCatgory::Median],
                  2e5);
  EXPECT_FLOAT_EQ(parser.snap_score_params_.vehicle_size_params->size_max
                      [SizeCatgory::Large],
                  1e6);

  EXPECT_FLOAT_EQ(parser.snap_score_params_.plate_size_params->weight, 3);
  EXPECT_EQ(parser.snap_score_params_.plate_size_params->norm_fun,
            NormFunType::Sigmoid);
  EXPECT_EQ(parser.snap_score_params_.plate_size_params->valid_min_w, 25);
  EXPECT_EQ(parser.snap_score_params_.plate_size_params->valid_min_h, 10);
  EXPECT_FLOAT_EQ(parser.snap_score_params_.plate_size_params->size_max
                [SizeCatgory::Unknown_Size],
            2.5e3);

  EXPECT_FLOAT_EQ(parser.snap_score_params_.vehicle_det_params->weight, 4);
  EXPECT_EQ(parser.snap_score_params_.vehicle_det_params->norm_fun,
            NormFunType::Sigmoid);
  EXPECT_FLOAT_EQ(parser.snap_score_params_.vehicle_det_params->min_value,
                  0.52);
  EXPECT_FLOAT_EQ(parser.snap_score_params_.vehicle_det_params->max_value, 1.9);

  EXPECT_FLOAT_EQ(parser.snap_score_params_.plate_det_params->weight, 5);
  EXPECT_EQ(parser.snap_score_params_.plate_det_params->norm_fun,
            NormFunType::Sigmoid);
  EXPECT_FLOAT_EQ(parser.snap_score_params_.plate_det_params->min_value, 0.88);
  EXPECT_FLOAT_EQ(parser.snap_score_params_.plate_det_params->max_value, 1.7);

  EXPECT_FLOAT_EQ(parser.snap_score_params_.visibility_params->weight, 7);
  EXPECT_EQ(parser.snap_score_params_.visibility_params->norm_fun,
            NormFunType::Sigmoid);

  EXPECT_FLOAT_EQ(parser.snap_score_params_.integrity_params->weight, 9);
  EXPECT_EQ(parser.snap_score_params_.integrity_params->dis_th, 27);
  EXPECT_EQ(parser.snap_score_params_.integrity_params->norm_fun,
            NormFunType::Sigmoid);

  EXPECT_FLOAT_EQ(parser.black_area_[0].x1, 1);
  EXPECT_FLOAT_EQ(parser.black_area_[0].y1, 2);
  EXPECT_FLOAT_EQ(parser.black_area_[0].x2, 3);
  EXPECT_FLOAT_EQ(parser.black_area_[0].y2, 4);
  EXPECT_FLOAT_EQ(parser.black_area_[1].x1, 5);
  EXPECT_FLOAT_EQ(parser.black_area_[1].y1, 6);
  EXPECT_FLOAT_EQ(parser.black_area_[1].x2, 7);
  EXPECT_FLOAT_EQ(parser.black_area_[1].y2, 8);
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
