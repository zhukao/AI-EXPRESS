/*
 * Copyright (c) 2019 Horizon Robotics
 * @author chao.yang
 * @date 2019.07.18
 */

#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_msg.hpp"
#include "horizon/vision_type/vision_type.hpp"
#include "horizon/vision_type/vision_type_util.h"
#include "gtest/gtest.h"

TEST(HorizonVisionSnapShotFrame, alloc_free) {
  HorizonVisionSnapshotFrame *snapshot_frame;
  auto ret = HorizonVisionAllocSnapshotFrame(&snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  int target_num = 20;
  ret = HorizonVisionAllocSnapshotTarget(&snapshot_frame->targets, target_num);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  snapshot_frame->targets_num = target_num;

  for (auto i = 0; i < target_num; ++i) {
    auto &target = snapshot_frame->targets[i];
    int snap_num = 3;
    ret = HorizonVisionAllocSnapshot(&target.snapshots, snap_num);
    EXPECT_EQ(kHorizonVisionSuccess, ret);
    target.snapshots_num = snap_num;
    ret = HorizonVisionAllocSmartData(&target.snapshots[0].smart_data, 1);
    EXPECT_EQ(kHorizonVisionSuccess, ret);
    target.snapshots[0].smart_data->track_id = 99;
  }

  ASSERT_TRUE(snapshot_frame->targets);
  ASSERT_TRUE(snapshot_frame->targets[0].snapshots);
  ASSERT_TRUE(snapshot_frame->targets[0].snapshots[0].smart_data);
  EXPECT_EQ(snapshot_frame->targets[0].snapshots[0].smart_data->track_id,
            size_t(99));
  ret = HorizonVisionFreeSnapshotFrame(snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionSnapShotFrame, copy) {
  HorizonVisionSnapshotFrame *snapshot_frame;
  auto ret = HorizonVisionAllocSnapshotFrame(&snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  snapshot_frame->targets_num = 2;
  ret = HorizonVisionAllocSnapshotTarget(
      &snapshot_frame->targets, snapshot_frame->targets_num);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  for (uint32_t i = 0; i < snapshot_frame->targets_num; i++) {
    auto &target = snapshot_frame->targets[i];
    target.snapshots_num = 3;
    ret = HorizonVisionAllocSnapshot(&target.snapshots, target.snapshots_num);
    EXPECT_EQ(kHorizonVisionSuccess, ret);
    for (uint32_t ii = 0; ii < target.snapshots_num; ii++) {
      auto &snapshot = target.snapshots[ii];
      snapshot.time_stamp = 26000000 + 2000 * ii;
      ret = HorizonVisionAllocSmartData(&snapshot.smart_data, 1);
      EXPECT_EQ(kHorizonVisionSuccess, ret);

      auto &smart_data = *snapshot.smart_data;
      smart_data.track_id = i;
      ret = HorizonVisionAllocBodySmartData(&smart_data.body);
      EXPECT_EQ(kHorizonVisionSuccess, ret);
      smart_data.body->body_rect.x1 = 1;
      smart_data.body->body_rect.y1 = 1;
      smart_data.body->body_rect.x2 = 2;
      smart_data.body->body_rect.y2 = 2;
      ret = HorizonVisionAllocFaceSmartData(&smart_data.face);
      EXPECT_EQ(kHorizonVisionSuccess, ret);
      smart_data.face->face_rect.x1 = 3;
      smart_data.face->face_rect.y1 = 3;
      smart_data.face->face_rect.x2 = 4;
      smart_data.face->face_rect.y2 = 4;

      ret = HorizonVisionAllocImage(&snapshot.croped_image);
      EXPECT_EQ(kHorizonVisionSuccess, ret);

      auto &image = *snapshot.croped_image;
      snapshot.croped_image->pixel_format =
          HorizonVisionPixelFormat::kHorizonVisionPixelFormatRawNV12;
      image.height = 128;
      image.width = 128;
      image.data_size = 128 * 128 * 3 / 2;
      image.stride = 128;
      image.stride_uv = 128;
      image.data = static_cast<uint8_t *>(
          std::calloc(image.data_size, sizeof(uint8_t)));
      image.data[0] = 100;
      image.data[1] = 101;
      image.data[2] = 102;
    }
  }

  HorizonVisionSnapshotFrame *cp_snapshot_frame;
  ret = HorizonVisionAllocSnapshotFrame(&cp_snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionCopySnapshotFrame(snapshot_frame, cp_snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // release original snapshot_frame data before checking copy data
  ret = HorizonVisionFreeSnapshotFrame(snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // checking copy data
  EXPECT_EQ(cp_snapshot_frame->targets_num, 2u);
  for (uint32_t i = 0; i < cp_snapshot_frame->targets_num; i++) {
    auto &target = cp_snapshot_frame->targets[i];
    EXPECT_EQ(target.snapshots_num, 3u);
    for (uint32_t ii = 0; ii < target.snapshots_num; ii++) {
      auto &snapshot = target.snapshots[ii];
      uint32_t time_stamp_gt = 26000000 + 2000 * ii;
      EXPECT_EQ(snapshot.time_stamp, time_stamp_gt);
      auto &smart_data = *snapshot.smart_data;
      EXPECT_EQ(smart_data.track_id, i);
      EXPECT_EQ(smart_data.body->body_rect.x1, 1.0f);
      EXPECT_EQ(smart_data.body->body_rect.y1, 1.0f);
      EXPECT_EQ(smart_data.body->body_rect.x2, 2.0f);
      EXPECT_EQ(smart_data.body->body_rect.y2, 2.0f);
      EXPECT_EQ(smart_data.face->face_rect.x1, 3.0f);
      EXPECT_EQ(smart_data.face->face_rect.y1, 3.0f);
      EXPECT_EQ(smart_data.face->face_rect.x2, 4.0f);
      EXPECT_EQ(smart_data.face->face_rect.y2, 4.0f);

      auto &image = *snapshot.croped_image;
      EXPECT_EQ(snapshot.croped_image->pixel_format,
                HorizonVisionPixelFormat::kHorizonVisionPixelFormatRawNV12);
      EXPECT_EQ(image.height, 128u);
      EXPECT_EQ(image.width, 128u);
      EXPECT_EQ(image.data_size, 128 * 128 * 3 / 2u);
      EXPECT_EQ(image.stride, 128u);
      EXPECT_EQ(image.stride_uv, 128u);
      EXPECT_EQ(image.data[0], 100);
      EXPECT_EQ(image.data[1], 101);
      EXPECT_EQ(image.data[2], 102);
    }
  }

  ret = HorizonVisionFreeSnapshotFrame(cp_snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionSnapShotFrame, copy_empty_target_data) {
  HorizonVisionSnapshotFrame *snapshot_frame;
  auto ret = HorizonVisionAllocSnapshotFrame(&snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  snapshot_frame->targets_num = 0;
  ret = HorizonVisionAllocSnapshotTarget(
      &snapshot_frame->targets, snapshot_frame->targets_num);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  HorizonVisionSnapshotFrame *cp_snapshot_frame;
  ret = HorizonVisionAllocSnapshotFrame(&cp_snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionCopySnapshotFrame(snapshot_frame, cp_snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // release original snapshot_frame data before checking copy data
  ret = HorizonVisionFreeSnapshotFrame(snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // checking copy data
  EXPECT_EQ(cp_snapshot_frame->targets_num, 0u);
  ret = HorizonVisionFreeSnapshotFrame(cp_snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionSnapShotFrame, copy_empty_snapshot) {
  HorizonVisionSnapshotFrame *snapshot_frame;
  auto ret = HorizonVisionAllocSnapshotFrame(&snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  snapshot_frame->targets_num = 2;
  ret = HorizonVisionAllocSnapshotTarget(
      &snapshot_frame->targets, snapshot_frame->targets_num);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  for (uint32_t i = 0; i < snapshot_frame->targets_num; i++) {
    auto &target = snapshot_frame->targets[i];
    target.snapshots_num = 0;
    ret = HorizonVisionAllocSnapshot(&target.snapshots, target.snapshots_num);
    EXPECT_EQ(kHorizonVisionSuccess, ret);
  }

  HorizonVisionSnapshotFrame *cp_snapshot_frame;
  ret = HorizonVisionAllocSnapshotFrame(&cp_snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionCopySnapshotFrame(snapshot_frame, cp_snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // release original snapshot_frame data before checking copy data
  ret = HorizonVisionFreeSnapshotFrame(snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // checking copy data
  EXPECT_EQ(cp_snapshot_frame->targets_num, 2u);
  for (uint32_t i = 0; i < cp_snapshot_frame->targets_num; i++) {
    auto &target = cp_snapshot_frame->targets[i];
    EXPECT_EQ(target.snapshots_num, 0u);
  }

  ret = HorizonVisionFreeSnapshotFrame(cp_snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionSnapShotFrame, dup) {
  HorizonVisionSnapshotFrame *snapshot_frame;
  auto ret = HorizonVisionAllocSnapshotFrame(&snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  snapshot_frame->targets_num = 2;
  ret = HorizonVisionAllocSnapshotTarget(
      &snapshot_frame->targets, snapshot_frame->targets_num);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  for (uint32_t i = 0; i < snapshot_frame->targets_num; i++) {
    auto &target = snapshot_frame->targets[i];
    target.snapshots_num = 3;
    ret = HorizonVisionAllocSnapshot(&target.snapshots, target.snapshots_num);
    EXPECT_EQ(kHorizonVisionSuccess, ret);
    for (uint32_t ii = 0; ii < target.snapshots_num; ii++) {
      auto &snapshot = target.snapshots[ii];
      snapshot.time_stamp = 26000000 + 2000 * ii;
      ret = HorizonVisionAllocSmartData(&snapshot.smart_data, 1);
      EXPECT_EQ(kHorizonVisionSuccess, ret);

      auto &smart_data = *snapshot.smart_data;
      smart_data.track_id = i;
      ret = HorizonVisionAllocBodySmartData(&smart_data.body);
      EXPECT_EQ(kHorizonVisionSuccess, ret);
      smart_data.body->body_rect.x1 = 1;
      smart_data.body->body_rect.y1 = 1;
      smart_data.body->body_rect.x2 = 2;
      smart_data.body->body_rect.y2 = 2;
      ret = HorizonVisionAllocFaceSmartData(&smart_data.face);
      EXPECT_EQ(kHorizonVisionSuccess, ret);
      smart_data.face->face_rect.x1 = 3;
      smart_data.face->face_rect.y1 = 3;
      smart_data.face->face_rect.x2 = 4;
      smart_data.face->face_rect.y2 = 4;

      ret = HorizonVisionAllocImage(&snapshot.croped_image);
      EXPECT_EQ(kHorizonVisionSuccess, ret);

      auto &image = *snapshot.croped_image;
      snapshot.croped_image->pixel_format =
          HorizonVisionPixelFormat::kHorizonVisionPixelFormatRawNV12;
      image.height = 128;
      image.width = 128;
      image.data_size = 128 * 128 * 3 / 2;
      image.stride = 128;
      image.stride_uv = 128;
      image.data = static_cast<uint8_t *>(
          std::calloc(image.data_size, sizeof(uint8_t)));
      image.data[0] = 100;
      image.data[1] = 101;
      image.data[2] = 102;
    }
  }

  HorizonVisionSnapshotFrame *dup_snapshot_frame;
  ret = HorizonVisionDupSnapshotFrame(snapshot_frame, &dup_snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // release original snapshot_frame data before checking copy data
  ret = HorizonVisionFreeSnapshotFrame(snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // checking copy data
  EXPECT_EQ(dup_snapshot_frame->targets_num, 2u);
  for (uint32_t i = 0; i < dup_snapshot_frame->targets_num; i++) {
    auto &target = dup_snapshot_frame->targets[i];
    EXPECT_EQ(target.snapshots_num, 3u);
    for (uint32_t ii = 0; ii < target.snapshots_num; ii++) {
      auto &snapshot = target.snapshots[ii];
      uint32_t time_stamp_gt = 26000000 + 2000 * ii;
      EXPECT_EQ(snapshot.time_stamp, time_stamp_gt);
      auto &smart_data = *snapshot.smart_data;
      EXPECT_EQ(smart_data.track_id, i);
      EXPECT_EQ(smart_data.body->body_rect.x1, 1.0f);
      EXPECT_EQ(smart_data.body->body_rect.y1, 1.0f);
      EXPECT_EQ(smart_data.body->body_rect.x2, 2.0f);
      EXPECT_EQ(smart_data.body->body_rect.y2, 2.0f);
      EXPECT_EQ(smart_data.face->face_rect.x1, 3.0f);
      EXPECT_EQ(smart_data.face->face_rect.y1, 3.0f);
      EXPECT_EQ(smart_data.face->face_rect.x2, 4.0f);
      EXPECT_EQ(smart_data.face->face_rect.y2, 4.0f);

      auto &image = *snapshot.croped_image;
      EXPECT_EQ(snapshot.croped_image->pixel_format,
                HorizonVisionPixelFormat::kHorizonVisionPixelFormatRawNV12);
      EXPECT_EQ(image.height, 128u);
      EXPECT_EQ(image.width, 128u);
      EXPECT_EQ(image.data_size, 128 * 128 * 3 / 2u);
      EXPECT_EQ(image.stride, 128u);
      EXPECT_EQ(image.stride_uv, 128u);
      EXPECT_EQ(image.data[0], 100);
      EXPECT_EQ(image.data[1], 101);
      EXPECT_EQ(image.data[2], 102);
    }
  }

  ret = HorizonVisionFreeSnapshotFrame(dup_snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionSnapShotFrame, dup_empty_target_data) {
  HorizonVisionSnapshotFrame *snapshot_frame;
  auto ret = HorizonVisionAllocSnapshotFrame(&snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  snapshot_frame->targets_num = 0;
  ret = HorizonVisionAllocSnapshotTarget(
      &snapshot_frame->targets, snapshot_frame->targets_num);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  HorizonVisionSnapshotFrame *cp_snapshot_frame;
  ret = HorizonVisionDupSnapshotFrame(snapshot_frame, &cp_snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // release original snapshot_frame data before checking copy data
  ret = HorizonVisionFreeSnapshotFrame(snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // checking copy data
  EXPECT_EQ(cp_snapshot_frame->targets_num, 0u);
  ret = HorizonVisionFreeSnapshotFrame(cp_snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionSnapShotFrame, dup_empty_snapshot) {
  HorizonVisionSnapshotFrame *snapshot_frame;
  auto ret = HorizonVisionAllocSnapshotFrame(&snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  snapshot_frame->targets_num = 2;
  ret = HorizonVisionAllocSnapshotTarget(
      &snapshot_frame->targets, snapshot_frame->targets_num);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  for (uint32_t i = 0; i < snapshot_frame->targets_num; i++) {
    auto &target = snapshot_frame->targets[i];
    target.snapshots_num = 0;
    ret = HorizonVisionAllocSnapshot(&target.snapshots, target.snapshots_num);
    EXPECT_EQ(kHorizonVisionSuccess, ret);
  }

  HorizonVisionSnapshotFrame *cp_snapshot_frame;
  ret = HorizonVisionDupSnapshotFrame(snapshot_frame, &cp_snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // release original snapshot_frame data before checking copy data
  ret = HorizonVisionFreeSnapshotFrame(snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  // checking copy data
  EXPECT_EQ(cp_snapshot_frame->targets_num, 2u);
  for (uint32_t i = 0; i < cp_snapshot_frame->targets_num; i++) {
    auto &target = cp_snapshot_frame->targets[i];
    EXPECT_EQ(target.snapshots_num, 0u);
  }

  ret = HorizonVisionFreeSnapshotFrame(cp_snapshot_frame);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}
