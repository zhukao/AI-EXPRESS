/*
 * Copyright (c) 2019 Horizon Robotics
 * @author chao.yang
 * @date 2019.07.18
 */

#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_type_util.h"
#include "gtest/gtest.h"

TEST(HorizonVisionSnapsShot, alloc_free) {
  HorizonVisionSnapshot *snap_list = nullptr;
  int snap_num = 3;
  auto res = HorizonVisionAllocSnapshot(&snap_list, snap_num);
  EXPECT_EQ(kHorizonVisionSuccess, res);

  for (auto i = 0; i < snap_num; ++i) {
    auto &snap = snap_list[i];
    res = HorizonVisionAllocSmartData(&snap.smart_data, 1);
    EXPECT_EQ(kHorizonVisionSuccess, res);
    res = HorizonVisionAllocImage(&snap.croped_image);
    EXPECT_EQ(kHorizonVisionSuccess, res);
    EXPECT_EQ(snap.smart_data->track_id, 0u);
    EXPECT_EQ(snap.croped_image->data_size, 0u);
  }
  res = HorizonVisionFreeSnapshot(snap_list, snap_num);
  EXPECT_EQ(kHorizonVisionSuccess, res);
}
