/*
 * Copyright (c) 2019 Horizon Robotics
 * @author chao.yang
 * @date 2019.07.18
 */

#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_type_util.h"
#include "gtest/gtest.h"

TEST(HorizonVisionImageFrame, alloc_free) {
  HorizonVisionImageFrame *data;
  auto ret = HorizonVisionAllocImageFrame(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionFreeImageFrame(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionImageFrame, dup_imgs) {
  int frame_num = 2;
  HorizonVisionImageFrame **frames = nullptr;
  auto ret = HorizonVisionAllocImageFrames(&frames, frame_num);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  for (auto i = 0; i < frame_num; ++i) {
    auto ret = HorizonVisionAllocImageFrame(&frames[i]);
    EXPECT_EQ(kHorizonVisionSuccess, ret);
  }
  // test dup with data
  HorizonVisionImageFrame **dup_frames = nullptr;
  ret = HorizonVisionDupImageFrames(frames, frame_num, &dup_frames);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  ret = HorizonVisionFreeImageFrames(dup_frames, frame_num);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  // test dup without data
  HorizonVisionImageFrame **dup_frames_without_data = nullptr;
  ret = HorizonVisionDupImageFramesWithoutData(frames,
                                               frame_num,
                                               &dup_frames_without_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionFreeImageFramesWithoutData(dup_frames_without_data,
                                                frame_num);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  HorizonVisionFreeImageFrames(frames, frame_num);
}



