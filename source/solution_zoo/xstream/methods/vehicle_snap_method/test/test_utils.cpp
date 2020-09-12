/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.16
 */

#include "gtest/gtest.h"
#include "vehicle_common_utility/data_type.hpp"
#include "vehicle_common_utility/utils.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

TEST(DATA_TYPE, BBOX) {
  std::vector<float> bbox_list{23, 45, 89, 90};
  vision::BBox bbox1(23, 45, 89, 90);
  EXPECT_FLOAT_EQ(bbox1.x1, 23);
  EXPECT_FLOAT_EQ(bbox1.y1, 45);
  EXPECT_FLOAT_EQ(bbox1.x2, 89);
  EXPECT_FLOAT_EQ(bbox1.y2, 90);

  vision::BBox bbox2(bbox_list[0], bbox_list[1], bbox_list[2], bbox_list[3]);
  EXPECT_FLOAT_EQ(bbox2.x1, 23);
  EXPECT_FLOAT_EQ(bbox2.y1, 45);
  EXPECT_FLOAT_EQ(bbox2.x2, 89);
  EXPECT_FLOAT_EQ(bbox2.y2, 90);
}

TEST(UTILS_FUNCTION, CENTER_X) {
  vision::BBox bbox(10, 20, 30, 40);
  float value = BBoxCenterX(bbox);
  EXPECT_FLOAT_EQ(value, 20);
}

TEST(UTILS_FUNCTION, CENTER_Y) {
  vision::BBox bbox(10, 20, 30, 40);
  float value = BBoxCenterY(bbox);
  EXPECT_FLOAT_EQ(value, 30);
}

TEST(UTILS_FUNCTION, AREA) {
  vision::BBox bbox(10, 20, 30, 40);
  float value = BBoxArea(bbox);
  EXPECT_FLOAT_EQ(value, 400);
}

TEST(UTILS_FUNCTION, INTERSECTION_AREA) {
  vision::BBox bbox1(0, 1, 20, 30);
  vision::BBox bbox2(10, 15, 40, 60);
  float value = IntersectionArea(bbox1, bbox2);
  EXPECT_FLOAT_EQ(value, 150);
}

TEST(UTILS_FUNCTION, BBOX_IN_RATIO) {
  vision::BBox bbox1(0, 1, 20, 30);
  vision::BBox bbox2(10, 15, 40, 60);
  float value = BBoxInRatio(bbox1, bbox2);
  EXPECT_FLOAT_EQ(value, 15 / 58.0);
}

TEST(UTILS_FUNCTION, BBOX_OVERLAP) {
  vision::BBox bbox1(0, 1, 20, 30);
  vision::BBox bbox2(10, 15, 40, 60);
  float value = BBoxOverlapRatio(bbox1, bbox2);
  EXPECT_FLOAT_EQ(value, 15 / 178.0);
}

TEST(UTILS_FUNCTION, BBOX_DISTANCE) {
  vision::BBox bbox1(0, 1, 20, 30);
  vision::BBox bbox2(10, 15, 40, 60);
  float value = BBoxDistance(bbox1, bbox2);
  EXPECT_FLOAT_EQ(value, 26.627053911388696);
}

TEST(UTILS_FUNCTION, SigmoidFunction) {
  EXPECT_FLOAT_EQ(SigmoidFunction(0.f), 0.5);
  EXPECT_FLOAT_EQ(SigmoidFunction(-5), 0.006692850924);
  EXPECT_FLOAT_EQ(SigmoidFunction(5), 0.993307);
  EXPECT_FLOAT_EQ(SigmoidFunction(0.458), 0.6125396134);
}

TEST(UTILS_FUNCTION, LinearFunction) {
  EXPECT_FLOAT_EQ(LinearFunction(0, -5, 5), 0.5);
  EXPECT_FLOAT_EQ(LinearFunction(-5, -5, 5), 0);
  EXPECT_FLOAT_EQ(LinearFunction(5, -5, 5), 1);
  EXPECT_FLOAT_EQ(LinearFunction(2, -5, 5), 0.7);
}

TEST(UTILS_FUNCTION, MapFunction) {
  EXPECT_FLOAT_EQ(MapFunction(0.f, NormFunType::Sigmoid, -5, 5), 0.5);
  EXPECT_FLOAT_EQ(MapFunction(0.f, NormFunType::Linear, -5, 5), 0.5);
  EXPECT_FLOAT_EQ(MapFunction(2.f, NormFunType::Linear, -5, 5), 0.7);
}

TEST(UTILS_FUNCTION, DistanceToLine) {
  std::pair<PointInt, PointInt> line{PointInt(1, 2), PointInt(5, 8)};
  EXPECT_FLOAT_EQ(DistanceToLine(PointInt(0, 0), line), 0.27735);
  EXPECT_FLOAT_EQ(DistanceToLine(PointInt(-1, -1), line), 0.f);
  EXPECT_FLOAT_EQ(DistanceToLine(PointInt(-1, 9), line), -5.54700);
}

TEST(UTILS_FUNTION, ArgMax) {
  DoubleVec double_vec1 =
      {{0, 0, 1, 0}, {0, 1, 0, 1}, {1, 1, 2, 0}, {0, 0, 0, 0}};
  DoubleVec double_vec2 =
      {{0, 3, 1, 0}, {0, 1, 0, 1}, {1, 1, 2, 0}, {0, 0, 0, 0}};
  DoubleVec double_vec3 =
      {{0, 0, 1, 0}, {0, 1, 0, 1}, {1, 1, 2, 0}, {0, 0, 4, 0}};
  TrebleVec treble_vec = {double_vec1, double_vec2, double_vec3};
  std::vector<int> double_max_idx{2, 2};
  std::vector<int> treble_max_idx{2, 3, 2};
  auto ret1 = ArgMax(double_vec1);
  EXPECT_EQ(double_max_idx.size(), ret1.size());
  EXPECT_EQ(double_max_idx[0], ret1[0]);
  EXPECT_EQ(double_max_idx[1], ret1[1]);
  auto ret2 = ArgMax(treble_vec);
  EXPECT_EQ(treble_max_idx.size(), ret2.size());
  EXPECT_EQ(treble_max_idx[0], ret2[0]);
  EXPECT_EQ(treble_max_idx[1], ret2[1]);
  EXPECT_EQ(treble_max_idx[2], ret2[2]);
}

TEST(UTILS_FUNCTION, UnravelIndex) {
  int index = 3;
  std::vector<int> shape{4, 3, 2, 1};
  auto ret = UnravelIndex(index, shape);
  EXPECT_EQ(ret.size(), shape.size());
  EXPECT_EQ(ret[0], 0);
  EXPECT_EQ(ret[1], 1);
  EXPECT_EQ(ret[2], 1);
  EXPECT_EQ(ret[3], 0);
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
