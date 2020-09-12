/*
 * Copyright (c) 2019 Horizon Robotics
 * @author chao.yang
 * @date 2019.07.18
 */

#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_type_util.h"
#include "gtest/gtest.h"

TEST(HorizonVisionLandmarks, alloc_free) {
  HorizonVisionLandmarks *data;
  auto ret = HorizonVisionAllocLandmarks(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  data->num = 5;
  data->score = 0.1;
  data->points = static_cast<HorizonVisionPoint *>(
      std::calloc(data->num, sizeof(HorizonVisionPoint)));
  for (size_t i = 0; i < data->num; ++i) {
    data->points[i].x = 1 * i;
    data->points[i].y = 2 * i;
    data->points[i].score = 0.5;
  }
  ret = HorizonVisionFreeLandmarks(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionLandmarks, alloc_free_empty_data) {
  HorizonVisionLandmarks *data;
  auto ret = HorizonVisionAllocLandmarks(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  data->num = 0;
  data->score = 0.1;
  data->points = static_cast<HorizonVisionPoint *>(
      std::calloc(data->num, sizeof(HorizonVisionPoint)));
  ret = HorizonVisionFreeLandmarks(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionLandmarks, cp) {
  HorizonVisionLandmarks *data;
  auto ret = HorizonVisionAllocLandmarks(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  data->num = 5;
  data->score = 0.1;
  data->points = static_cast<HorizonVisionPoint *>(
      std::calloc(data->num, sizeof(HorizonVisionPoint)));
  for (size_t i = 0; i < data->num; ++i) {
    data->points[i].x = 1 * i;
    data->points[i].y = 2 * i;
    data->points[i].score = 0.5;
  }

  HorizonVisionLandmarks *cp_data;
  ret = HorizonVisionAllocLandmarks(&cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionCopyLandmarks(data, cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // release original data before checking copy data
  ret = HorizonVisionFreeLandmarks(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  EXPECT_EQ(cp_data->num, 5u);
  EXPECT_EQ(cp_data->score, 0.1f);
  for (size_t i = 0; i < cp_data->num; ++i) {
    EXPECT_EQ(cp_data->points[i].x, 1 * i);
    EXPECT_EQ(cp_data->points[i].y, 2 * i);
    EXPECT_EQ(cp_data->points[i].score, 0.5f);
  }
  ret = HorizonVisionFreeLandmarks(cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionLandmarks, cp_empty_data) {
  HorizonVisionLandmarks *data;
  auto ret = HorizonVisionAllocLandmarks(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  data->num = 0;
  data->score = 0.1;

  HorizonVisionLandmarks *cp_data;
  ret = HorizonVisionAllocLandmarks(&cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionCopyLandmarks(data, cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // release original data before checking copy data
  ret = HorizonVisionFreeLandmarks(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  EXPECT_EQ(cp_data->num, 0u);
  EXPECT_EQ(cp_data->score, 0.1f);
  ret = HorizonVisionFreeLandmarks(cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionLandmarks, dup) {
  HorizonVisionLandmarks *data;
  auto ret = HorizonVisionAllocLandmarks(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  data->num = 5;
  data->score = 0.1;
  data->points = static_cast<HorizonVisionPoint *>(
      std::calloc(data->num, sizeof(HorizonVisionPoint)));
  for (size_t i = 0; i < data->num; ++i) {
    data->points[i].x = 1 * i;
    data->points[i].y = 2 * i;
    data->points[i].score = 0.5;
  }

  HorizonVisionLandmarks *cp_data;
  ret = HorizonVisionDupLandmarks(data, &cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // release original data before checking copy data
  ret = HorizonVisionFreeLandmarks(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  EXPECT_EQ(cp_data->num, 5u);
  EXPECT_EQ(cp_data->score, 0.1f);
  for (size_t i = 0; i < cp_data->num; ++i) {
    EXPECT_EQ(cp_data->points[i].x, 1 * i);
    EXPECT_EQ(cp_data->points[i].y, 2 * i);
    EXPECT_EQ(cp_data->points[i].score, 0.5f);
  }
  ret = HorizonVisionFreeLandmarks(cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionLandmarks, dup_empty_data) {
  HorizonVisionLandmarks *data;
  auto ret = HorizonVisionAllocLandmarks(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  data->num = 0;
  data->score = 0.1;

  HorizonVisionLandmarks *cp_data;
  ret = HorizonVisionDupLandmarks(data, &cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // release original data before checking copy data
  ret = HorizonVisionFreeLandmarks(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  EXPECT_EQ(cp_data->num, 0u);
  EXPECT_EQ(cp_data->score, 0.1f);
  ret = HorizonVisionFreeLandmarks(cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}
