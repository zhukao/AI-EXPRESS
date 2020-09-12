/*
 * Copyright (c) 2019 Horizon Robotics
 * @author chao.yang
 * @date 2019.07.18
 */

#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_type_util.h"
#include "gtest/gtest.h"

TEST(HorizonVisionFloatArray, alloc_free) {
  HorizonVisionFloatArray *data;
  auto ret = HorizonVisionAllocFloatArray(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  data->num = 2;
  data->values = static_cast<float *>(std::calloc(2, sizeof(float)));
  data->values[0] = 0.3;
  data->values[1] = 0.5;
  ret = HorizonVisionFreeFloatArray(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionFloatArray, alloc_free_empty_data) {
  HorizonVisionFloatArray *data;
  auto ret = HorizonVisionAllocFloatArray(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionFreeFloatArray(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionFloatArray, copy) {
  HorizonVisionFloatArray *data;
  auto ret = HorizonVisionAllocFloatArray(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  data->num = 2;
  data->values = static_cast<float *>(std::calloc(2, sizeof(float)));
  data->values[0] = 0.3;
  data->values[1] = 0.5;

  HorizonVisionFloatArray *cp_data;
  ret = HorizonVisionAllocFloatArray(&cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionCopyFloatArray(data, cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // release original data before checking
  ret = HorizonVisionFreeFloatArray(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  EXPECT_EQ(cp_data->num, 2u);
  EXPECT_EQ(cp_data->values[0], 0.3f);
  EXPECT_EQ(cp_data->values[1], 0.5f);
  ret = HorizonVisionFreeFloatArray(cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionFloatArray, copy_empty_data) {
  HorizonVisionFloatArray *data;
  auto ret = HorizonVisionAllocFloatArray(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  HorizonVisionFloatArray *cp_data;
  ret = HorizonVisionAllocFloatArray(&cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionCopyFloatArray(data, cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  ret = HorizonVisionFreeFloatArray(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionFreeFloatArray(cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionFloatArray, dup) {
  HorizonVisionFloatArray *data;
  auto ret = HorizonVisionAllocFloatArray(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  data->num = 2;
  data->values = static_cast<float *>(std::calloc(2, sizeof(float)));
  data->values[0] = 0.3;
  data->values[1] = 0.5;

  HorizonVisionFloatArray *cp_data;
  ret = HorizonVisionDupFloatArray(data, &cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // release original data before checking
  ret = HorizonVisionFreeFloatArray(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  EXPECT_EQ(cp_data->num, 2u);
  EXPECT_EQ(cp_data->values[0], 0.3f);
  EXPECT_EQ(cp_data->values[1], 0.5f);
  ret = HorizonVisionFreeFloatArray(cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionFloatArray, dup_empty_data) {
  HorizonVisionFloatArray *data;
  auto ret = HorizonVisionAllocFloatArray(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  HorizonVisionFloatArray *cp_data;
  ret = HorizonVisionDupFloatArray(data, &cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  ret = HorizonVisionFreeFloatArray(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionFreeFloatArray(cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}
