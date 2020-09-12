/*
 * Copyright (c) 2019 Horizon Robotics
 * @author chao.yang
 * @date 2019.07.18
 */

#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_type_util.h"
#include "gtest/gtest.h"

TEST(HorizonVisionSegmentation, alloc_free) {
  HorizonVisionSegmentation *data;
  auto ret = HorizonVisionAllocSegmentation(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  data->width = 10;
  data->height = 20;
  data->num = 2;
  data->values = static_cast<float *>(std::calloc(2, sizeof(float)));
  data->values[0] = 10.3;
  data->values[1] = 0.3;
  ret = HorizonVisionFreeSegmentation(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionSegmentation, alloc_free_empty_data) {
  HorizonVisionSegmentation *data;
  auto ret = HorizonVisionAllocSegmentation(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionFreeSegmentation(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionSegmentation, copy) {
  HorizonVisionSegmentation *data;
  auto ret = HorizonVisionAllocSegmentation(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  data->width = 10;
  data->height = 20;
  data->num = 2;
  data->values = static_cast<float *>(std::calloc(2, sizeof(float)));
  data->values[0] = 10.3;
  data->values[1] = 0.3;

  HorizonVisionSegmentation *cp_data;
  ret = HorizonVisionAllocSegmentation(&cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionCopySegmentation(data, cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // release original data before checking
  ret = HorizonVisionFreeSegmentation(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  EXPECT_EQ(cp_data->width, 10);
  EXPECT_EQ(cp_data->height, 20);
  EXPECT_EQ(cp_data->num, 2u);
  EXPECT_EQ(cp_data->values[0], 10.3f);
  EXPECT_EQ(cp_data->values[1], 0.3f);
  ret = HorizonVisionFreeSegmentation(cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionSegmentation, copy_empty_data) {
  HorizonVisionSegmentation *data;
  auto ret = HorizonVisionAllocSegmentation(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  HorizonVisionSegmentation *cp_data;
  ret = HorizonVisionAllocSegmentation(&cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionCopySegmentation(data, cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  ret = HorizonVisionFreeSegmentation(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionFreeSegmentation(cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionSegmentation, dup) {
  HorizonVisionSegmentation *data;
  auto ret = HorizonVisionAllocSegmentation(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  data->width = 10;
  data->height = 20;
  data->num = 2;
  data->values = static_cast<float *>(std::calloc(2, sizeof(float)));
  data->values[0] = 10.3;
  data->values[1] = 0.3;

  HorizonVisionSegmentation *cp_data;
  ret = HorizonVisionDupSegmentation(data, &cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // release original data before checking
  ret = HorizonVisionFreeSegmentation(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  EXPECT_EQ(cp_data->width, 10);
  EXPECT_EQ(cp_data->height, 20);
  EXPECT_EQ(cp_data->num, 2u);
  EXPECT_EQ(cp_data->values[0], 10.3f);
  EXPECT_EQ(cp_data->values[1], 0.3f);
  ret = HorizonVisionFreeSegmentation(cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionSegmentation, dup_empty_data) {
  HorizonVisionSegmentation *data;
  auto ret = HorizonVisionAllocSegmentation(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  HorizonVisionSegmentation *cp_data;
  ret = HorizonVisionDupSegmentation(data, &cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  ret = HorizonVisionFreeSegmentation(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionFreeSegmentation(cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}
