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

TEST(HorizonVisionSmartFrame, alloc_free) {
  HorizonVisionSmartFrame *data;
  auto ret = HorizonVisionAllocSmartFrame(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionFreeSmartFrame(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionSmartFrame, copy) {
  HorizonVisionSmartFrame *data;
  auto ret = HorizonVisionAllocSmartFrame(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  data->frame_id = 10;
  data->time_stamp = 23600000;
  data->image_num = 2;
  data->image_frame = static_cast<HorizonVisionImageFrame **>(
      std::calloc(data->image_num, sizeof(HorizonVisionImageFrame *)));

  for (uint32_t i = 0; i < data->image_num; i++) {
    HorizonVisionImageFrame *image_frame = nullptr;
    ret = HorizonVisionAllocImageFrame(&image_frame);
    EXPECT_EQ(kHorizonVisionSuccess, ret);
    image_frame->time_stamp = 23600000;
    image_frame->frame_id = 10;
    image_frame->channel_id = i;
    image_frame->image.pixel_format =
        HorizonVisionPixelFormat::kHorizonVisionPixelFormatRawNV12;
    image_frame->image.height = 1080;
    image_frame->image.width = 1920;
    image_frame->image.data_size = 1920 * 1080 * 3 / 2;
    image_frame->image.stride = 1920;
    image_frame->image.stride_uv = 1920;
    data->image_frame[i] = image_frame;
  }

  data->smart_data_list_num = 2;
  ret = HorizonVisionAllocSmartData(&data->smart_data_list, 2);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  for (uint32_t i = 0; i < data->smart_data_list_num; i++) {
    auto &smart_data = data->smart_data_list[i];
    smart_data.track_id = 5;
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
  }

  HorizonVisionSmartFrame *cp_data;
  ret = HorizonVisionAllocSmartFrame(&cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionCopySmartFrame(data, cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionFreeSmartFrame(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  EXPECT_EQ(cp_data->frame_id, 10u);
  EXPECT_EQ(cp_data->time_stamp, 23600000u);
  EXPECT_EQ(cp_data->image_num, 2u);

  for (uint32_t i = 0; i < cp_data->image_num; i++) {
    auto image_frame = cp_data->image_frame[i];
    EXPECT_EQ(image_frame->time_stamp, 23600000u);
    EXPECT_EQ(image_frame->frame_id, 10u);
    EXPECT_EQ(image_frame->channel_id, i);
    auto &image = image_frame->image;
    EXPECT_EQ(image.pixel_format, kHorizonVisionPixelFormatRawNV12);
    EXPECT_EQ(image.height, 1080u);
    EXPECT_EQ(image.width, 1920u);
    // data is nullptr ,so size is 0 too
    EXPECT_EQ(image.data_size, uint32_t(0));
    EXPECT_EQ(image.stride, 1920u);
    EXPECT_EQ(image.stride_uv, 1920u);
  }

  for (uint32_t i = 0; i < cp_data->smart_data_list_num; i++) {
    auto &smart_data = cp_data->smart_data_list[i];
    EXPECT_EQ(smart_data.track_id, 5u);
    EXPECT_EQ(smart_data.body->body_rect.x1, 1.0f);
    EXPECT_EQ(smart_data.body->body_rect.y1, 1.0f);
    EXPECT_EQ(smart_data.body->body_rect.x2, 2.0f);
    EXPECT_EQ(smart_data.body->body_rect.y2, 2.0f);
    EXPECT_EQ(smart_data.face->face_rect.x1, 3.0f);
    EXPECT_EQ(smart_data.face->face_rect.y1, 3.0f);
    EXPECT_EQ(smart_data.face->face_rect.x2, 4.0f);
    EXPECT_EQ(smart_data.face->face_rect.y2, 4.0f);
  }

  ret = HorizonVisionFreeSmartFrame(cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionSmartFrame, copy_empty_data) {
  HorizonVisionSmartFrame *data;
  auto ret = HorizonVisionAllocSmartFrame(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  data->frame_id = 10;
  data->time_stamp = 23600000;
  data->image_num = 0;
  data->smart_data_list_num = 0;
  ret = HorizonVisionAllocSmartData(&data->smart_data_list, 0);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  HorizonVisionSmartFrame *cp_data;
  ret = HorizonVisionAllocSmartFrame(&cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionCopySmartFrame(data, cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionFreeSmartFrame(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  EXPECT_EQ(cp_data->frame_id, 10u);
  EXPECT_EQ(cp_data->time_stamp, 23600000u);
  EXPECT_EQ(cp_data->image_num, 0u);
  EXPECT_EQ(cp_data->smart_data_list_num, 0u);
  ret = HorizonVisionFreeSmartFrame(cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionSmartFrame, dup) {
  HorizonVisionSmartFrame *data;
  auto ret = HorizonVisionAllocSmartFrame(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  data->frame_id = 10;
  data->time_stamp = 23600000;
  data->image_num = 2;
  ret = HorizonVisionAllocImageFrames(&data->image_frame, data->image_num);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  for (uint32_t i = 0; i < data->image_num; i++) {
    HorizonVisionImageFrame *image_frame = nullptr;
    ret = HorizonVisionAllocImageFrame(&image_frame);
    EXPECT_EQ(kHorizonVisionSuccess, ret);
    image_frame->time_stamp = 23600000;
    image_frame->frame_id = 10;
    image_frame->channel_id = i;
    image_frame->image.pixel_format =
        HorizonVisionPixelFormat::kHorizonVisionPixelFormatRawNV12;
    image_frame->image.height = 1080;
    image_frame->image.width = 1920;
    image_frame->image.data_size = 1920 * 1080 * 3 / 2;
    image_frame->image.stride = 1920;
    image_frame->image.stride_uv = 1920;
    data->image_frame[i] = image_frame;
  }

  data->smart_data_list_num = 2;
  ret = HorizonVisionAllocSmartData(&data->smart_data_list, 2);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  for (uint32_t i = 0; i < data->smart_data_list_num; i++) {
    auto &smart_data = data->smart_data_list[i];
    smart_data.track_id = 5;
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
  }
  // test duplicate smart frame
  HorizonVisionSmartFrame *dup_data;
  ret = HorizonVisionDupSmartFrame(data, &dup_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionFreeSmartFrame(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  EXPECT_EQ(dup_data->frame_id, 10u);
  EXPECT_EQ(dup_data->time_stamp, 23600000u);
  EXPECT_EQ(dup_data->image_num, 2u);

  for (uint32_t i = 0; i < dup_data->image_num; i++) {
    auto image_frame = dup_data->image_frame[i];
    EXPECT_EQ(image_frame->time_stamp, 23600000u);
    EXPECT_EQ(image_frame->frame_id, 10u);
    EXPECT_EQ(image_frame->channel_id, i);
    auto &image = image_frame->image;
    EXPECT_EQ(image.pixel_format, kHorizonVisionPixelFormatRawNV12);
    EXPECT_EQ(image.height, 1080u);
    EXPECT_EQ(image.width, 1920u);
    // because original data is nullptr ,so size is 0
    EXPECT_EQ(image.data_size, uint32_t(0));
    EXPECT_EQ(image.stride, 1920u);
    EXPECT_EQ(image.stride_uv, 1920u);
  }

  for (uint32_t i = 0; i < dup_data->smart_data_list_num; i++) {
    auto &smart_data = dup_data->smart_data_list[i];
    EXPECT_EQ(smart_data.track_id, 5u);
    EXPECT_EQ(smart_data.body->body_rect.x1, 1.0f);
    EXPECT_EQ(smart_data.body->body_rect.y1, 1.0f);
    EXPECT_EQ(smart_data.body->body_rect.x2, 2.0f);
    EXPECT_EQ(smart_data.body->body_rect.y2, 2.0f);
    EXPECT_EQ(smart_data.face->face_rect.x1, 3.0f);
    EXPECT_EQ(smart_data.face->face_rect.y1, 3.0f);
    EXPECT_EQ(smart_data.face->face_rect.x2, 4.0f);
    EXPECT_EQ(smart_data.face->face_rect.y2, 4.0f);
  }
  // test duplicate without data
  HorizonVisionSmartFrame *dup_data2;
  ret = HorizonVisionDupSmartFrameWithoutData(dup_data, &dup_data2);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionFreeSmartFrameWithoutData(dup_data2);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  ret = HorizonVisionFreeSmartFrame(dup_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}

TEST(HorizonVisionSmartFrame, dup_empty_data) {
  HorizonVisionSmartFrame *data;
  auto ret = HorizonVisionAllocSmartFrame(&data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  data->frame_id = 10;
  data->time_stamp = 23600000;
  data->image_num = 0;
  data->smart_data_list_num = 0;
  ret = HorizonVisionAllocSmartData(&data->smart_data_list, 0);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  HorizonVisionSmartFrame *cp_data;
  ret = HorizonVisionDupSmartFrame(data, &cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
  ret = HorizonVisionFreeSmartFrame(data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);

  EXPECT_EQ(cp_data->frame_id, 10u);
  EXPECT_EQ(cp_data->time_stamp, 23600000u);
  EXPECT_EQ(cp_data->image_num, 0u);
  EXPECT_EQ(cp_data->smart_data_list_num, 0u);
  std::free(cp_data->image_frame);
  ret = HorizonVisionFreeSmartFrame(cp_data);
  EXPECT_EQ(kHorizonVisionSuccess, ret);
}
