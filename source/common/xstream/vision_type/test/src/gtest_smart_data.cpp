/*
 * Copyright (c) 2019 Horizon Robotics
 * @author chao.yang
 * @date 2019.07.18
 */

#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_type_util.h"
#include "gtest/gtest.h"

TEST(HorizonVisionFaceSmartData, alloc_free) {
  HorizonVisionFaceSmartData *data;
  auto ret = HorizonVisionAllocFaceSmartData(&data);
  HorizonVisionAllocLandmarks(&data->landmarks);
  HorizonVisionAllocFloatArray(&data->feature);
  HorizonVisionAllocCharArray(&data->encrypted_feature);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionFreeFaceSmartData(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionBodySmartData, alloc_free) {
  HorizonVisionBodySmartData *data;
  auto ret = HorizonVisionAllocBodySmartData(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  HorizonVisionAllocSegmentation(&data->segmentation);
  HorizonVisionAllocLandmarks(&data->skeleton);
  ret = HorizonVisionFreeBodySmartData(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionSmartData, alloc_free) {
  HorizonVisionSmartData *data;
  auto ret = HorizonVisionAllocSmartData(&data, 1);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  EXPECT_EQ(data->type, uint(0));
  EXPECT_EQ(data->face_extra, nullptr);
  ret = HorizonVisionFreeSmartData(data, 1);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}
