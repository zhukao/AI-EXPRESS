/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.30
 */

#include <string>
#include "gtest/gtest.h"
#include "vehicle_snap_strategy/target_builder.hpp"
#include "vehicle_snap_strategy/size_score.hpp"
#include "vehicle_snap_strategy/det_conf_score.hpp"
#include "vehicle_snap_strategy/integrity_score.hpp"
#include "vehicle_snap_strategy/visibility_score.hpp"
#include "vehicle_snap_strategy/parse_config_from_json.hpp"
#include "vehicle_snap_strategy/vehicle_snap_strategy.hpp"
#include "utils_test.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

TEST(VehicleSnapStrategy, Basic) {
  std::string snap_config_file("./config/config_snap.json");
  VehicleSnapStrategy strategy = VehicleSnapStrategy(snap_config_file,
                                                    crop_image);
  SnapMode mode = SnapMode::CrossLineSnap;
  auto status = strategy.SetSnapMode(mode);
  EXPECT_EQ(status.value, StatusType::kOk);
  std::pair<PointInt, PointInt> line{PointInt(0, 100), PointInt(1000, 100)};
  status = strategy.SetLine(line);
  EXPECT_EQ(status.value, StatusType::kOk);
  vision::BBox box(20, 20, 40, 70);
  std::vector<vision::BBox> black_area{box};
  status = strategy.SetBlackArea(black_area);
  EXPECT_EQ(status.value, StatusType::kOk);
  std::ifstream cfg_file(snap_config_file);
  std::string cfg_str((std::istreambuf_iterator<char>(cfg_file)),
                      std::istreambuf_iterator<char>());
  status = strategy.UpdateSnapParams(cfg_str);
  EXPECT_EQ(status.value, StatusType::kOk);
}

TEST(VehicleSnapStrategy, TargetBuilder) {
  std::string snap_config_file("./config/config_snap.json");
  auto parser = ParseJsonSnap(snap_config_file);
  parser.snap_score_params_.vehicle_size_params->valid_min_h = 10;
  parser.snap_score_params_.vehicle_size_params->valid_min_w = 10;
  parser.snap_score_params_.plate_size_params->valid_min_h = 5;
  parser.snap_score_params_.plate_size_params->valid_min_w = 5;

  VehicleListPtr vehicle_list = std::make_shared<VehicleList>();

  ImageMetaType image_meta_info;
  image_meta_info.img_width = 128;
  image_meta_info.img_height = 128;
  image_meta_info.time_stamp = 40;

  vision::BBox tmp_bbox(90, 90, 128, 128);
  std::vector<vision::BBox> black_area;
  black_area.push_back(tmp_bbox);

  Vehicle vehicle1;
  vehicle1.bbox = std::make_shared<vision::BBox>(1, 2, 30, 40);
  vehicle1.bbox->score = 0.95;
  vehicle1.bbox->id = 1;
  vehicle_list->push_back(vehicle1);

  Vehicle vehicle2;
  vehicle2.bbox = std::make_shared<vision::BBox>(20, 25, 50, 55);
  vehicle2.bbox->score = 0.98;
  vehicle2.bbox->id = 2;
  vehicle2.plate = std::make_shared<Plate>();
  vehicle2.plate->bbox = std::make_shared<vision::BBox>(35, 40, 45, 50);
  vehicle2.plate->bbox->score = 0.8;
  vehicle_list->push_back(vehicle2);

  Vehicle vehicle3;
  vehicle3.bbox = std::make_shared<vision::BBox>(100, 100, 120, 120);
  vehicle3.bbox->score = 0.99;
  vehicle3.bbox->id = 3;
  vehicle_list->push_back(vehicle3);

  TargetBuilder target_builder(
      parser.snap_score_params_,
      parser.snap_post_control_params_.score_selective_snap_params);
  target_builder.SetBlackArea(black_area);
  target_builder.BuildTarget(image_meta_info, vehicle_list);

  // EXPECT_EQ((*vehicle_list)[0].is_valid, false);
  // EXPECT_EQ((*vehicle_list)[1].is_valid, true);
  // EXPECT_EQ((*vehicle_list)[2].is_valid, false);

  EXPECT_FLOAT_EQ((*vehicle_list)[0].occ_ratio, 150.0 / (29.0 * 38.0));
  EXPECT_FLOAT_EQ((*vehicle_list)[1].occ_ratio, 0);
}

TEST(VehicleSnapStrategy, SnapScore) {
  std::string snap_config_file("./config/config_snap.json");
  auto parser = ParseJsonSnap(snap_config_file);
  parser.snap_score_params_.vehicle_size_params->valid_min_h = 10;
  parser.snap_score_params_.vehicle_size_params->valid_min_w = 10;
  parser.snap_score_params_.vehicle_size_params->size_max
      [SizeCatgory::Unknown_Size] = 128.0 * 128.0 / 2.0;
  parser.snap_score_params_.plate_det_params->min_value = 0.7;
  parser.snap_score_params_.plate_size_params->valid_min_h = 5;
  parser.snap_score_params_.plate_size_params->valid_min_w = 5;
  parser.snap_score_params_.plate_size_params->size_max
      [SizeCatgory::Unknown_Size] = 128.0 * 128.0 / 128.0;

  VehicleListPtr vehicle_list = std::make_shared<VehicleList>();

  int img_width = 128;
  int img_height = 128;
  ImageMetaType image_meta_info;
  image_meta_info.img_width = img_width;
  image_meta_info.img_height = img_height;
  image_meta_info.time_stamp = 40;

  vision::BBox tmp_bbox(90, 90, 128, 128);
  std::vector<vision::BBox> black_area;
  black_area.push_back(tmp_bbox);

  Vehicle vehicle1;
  vehicle1.bbox = std::make_shared<vision::BBox>(1, 2, 30, 40);
  vehicle1.bbox->score = 0.95;
  vehicle1.bbox->id = 1;
  vehicle_list->push_back(vehicle1);

  Vehicle vehicle2;
  vehicle2.bbox = std::make_shared<vision::BBox>(20, 25, 50, 55);
  vehicle2.bbox->score = 0.98;
  vehicle2.bbox->id = 2;
  vehicle2.plate = std::make_shared<Plate>();
  vehicle2.plate->bbox = std::make_shared<vision::BBox>(35, 40, 45, 50);
  vehicle2.plate->bbox->score = 0.8;
  vehicle_list->push_back(vehicle2);

  TargetBuilder target_builder(
      parser.snap_score_params_,
      parser.snap_post_control_params_.score_selective_snap_params);
  target_builder.SetBlackArea(black_area);
  target_builder.BuildTarget(image_meta_info, vehicle_list);

  SizeScore vehicle_size_calculator(
      img_width, img_height, ObjectType::VEHICLE,
      parser.snap_score_params_.vehicle_size_params);
  SizeScore plate_size_calculator(img_width, img_height, ObjectType::PLATE,
                                  parser.snap_score_params_.plate_size_params);
  DetConfScore vehicle_det_score_calculator(
      ObjectType::VEHICLE, parser.snap_score_params_.vehicle_det_params);
  DetConfScore plate_det_score_calculator(
      ObjectType::PLATE, parser.snap_score_params_.plate_det_params);
  VisibilityScore visibility_score_calculator(
      parser.snap_score_params_.visibility_params);
  IntegrityScore integrity_score_calculator(
      img_width, img_height, parser.snap_score_params_.integrity_params);

  float score = vehicle_size_calculator.ComputeScore((*vehicle_list)[0]);
  EXPECT_FLOAT_EQ(score, 29.0 * 38.0 / (128.0 * 128.0 / 2.0));

  score = plate_size_calculator.ComputeScore((*vehicle_list)[1]);
  EXPECT_FLOAT_EQ(score, 10.0 * 10.0 / (128.0 * 128.0 / 128.0));
  // EXPECT_FLOAT_EQ(score, 1.f);

  score = vehicle_det_score_calculator.ComputeScore((*vehicle_list)[1]);
  EXPECT_FLOAT_EQ(score, 0.66666698);

  score = plate_det_score_calculator.ComputeScore((*vehicle_list)[1]);
  EXPECT_FLOAT_EQ(score, (0.8 - 0.7) / (1.0 - 0.7));

  score = integrity_score_calculator.ComputeScore((*vehicle_list)[0]);
  EXPECT_FLOAT_EQ(score, 0.f);

  score = integrity_score_calculator.ComputeScore((*vehicle_list)[1]);
  EXPECT_FLOAT_EQ(score, 1.f);

  score = visibility_score_calculator.ComputeScore((*vehicle_list)[0]);
  EXPECT_FLOAT_EQ(score, 1.f - 150.0 / (29.0 * 38.0));

  score = visibility_score_calculator.ComputeScore((*vehicle_list)[1]);
  EXPECT_FLOAT_EQ(score, 1.f);
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
