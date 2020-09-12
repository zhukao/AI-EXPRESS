/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     test class ImageCropper
 * @author    hangjun.yang
 * @email     hangjun.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.14
 */

#include <fstream>
#include <iostream>
#include "gtest/gtest.h"
#include "hobotxstream/imagetools/convertor.h"
#include "hobotxstream/imagetools/decoder.h"
#include "hobotxstream/imagetools/common.h"
#include "include/common.h"
#include "hobotxstream/image_tools.h"
#include "hobotxstream/imagetools/cropper.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

class CropperTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    std::ifstream istrm;
    istrm.open("test.jpg", std::ios::binary | std::ios::in);
    if (!istrm.good()) {
      return;
    }
    istrm.seekg(0, std::ios::end);
    int jpg_data_size = istrm.tellg();
    istrm.seekg(0, std::ios::beg);
    uint8_t *jpg_data = nullptr;
    HobotXStreamAllocImage(jpg_data_size, &jpg_data);
    istrm.read(reinterpret_cast<char *>(jpg_data), jpg_data_size);
    istrm.close();
    ImageDecoder decoder;
    bool ret = decoder.Decode(jpg_data,
                              jpg_data_size,
                              IMAGE_TOOLS_RAW_BGR,
                              bgr_data_);
    HOBOT_CHECK(ret != false);
    HobotXStreamFreeImage(jpg_data);

    ImageConvertor convertor;
    ret = convertor.Convert(bgr_data_, IMAGE_TOOLS_RAW_RGB, rgb_data_);
    HOBOT_CHECK(ret != false);
    ret = convertor.Convert(bgr_data_, IMAGE_TOOLS_RAW_GRAY, gray_data_);
    HOBOT_CHECK(ret != false);
    ret = convertor.Convert(bgr_data_, IMAGE_TOOLS_RAW_YUV_I420, i420_data_);
    HOBOT_CHECK(ret != false);
    ret = convertor.Convert(i420_data_, IMAGE_TOOLS_RAW_YUV_NV12, nv12_data_);
    HOBOT_CHECK(ret != false);
    ret = convertor.Convert(rgb_data_, IMAGE_TOOLS_RAW_YUV_NV21, nv21_data_);
    HOBOT_CHECK(ret != false);
  }

  virtual void TearDown() {
    HobotXStreamFreeImage(rgb_data_.data_[0]);
    HobotXStreamFreeImage(bgr_data_.data_[0]);
    HobotXStreamFreeImage(gray_data_.data_[0]);
    HobotXStreamFreeImage(i420_data_.data_[0]);
    HobotXStreamFreeImage(nv21_data_.data_[0]);
    HobotXStreamFreeImage(nv12_data_.data_[0]);
  }
  ImageToolsFormatData rgb_data_;
  ImageToolsFormatData bgr_data_;
  ImageToolsFormatData gray_data_;
  ImageToolsFormatData i420_data_;
  ImageToolsFormatData nv12_data_;
  ImageToolsFormatData nv21_data_;
  int crop_rect1_[4] = {10, 560, 601, 931};
  int crop_rect2_[4] = {11, 561, 602, 932};
  int width_ = 592;
  int height_ = 372;
};

TEST_F(CropperTest, Bgr_Normal) {
  ImageCropper cropper;
  bool ret = true;
  ImageToolsFormatData output1;
  ImageToolsFormatData output2;
  ImageToolsFormatData output1_i420;
  ImageToolsFormatData output2_i420;
  ret = cropper.Crop(bgr_data_,
                     crop_rect1_[0],
                     crop_rect1_[1],
                     crop_rect1_[2],
                     crop_rect1_[3],
                     output1);
  EXPECT_EQ(true, ret);

  ret = cropper.Crop(bgr_data_,
                     crop_rect2_[0],
                     crop_rect2_[1],
                     crop_rect2_[2],
                     crop_rect2_[3],
                     output2);
  EXPECT_EQ(true, ret);

  EXPECT_EQ(crop_rect1_[2] - crop_rect1_[0] + 1, output1.width_);
  EXPECT_EQ(crop_rect1_[3] - crop_rect1_[1] + 1, output1.height_);
  EXPECT_EQ(crop_rect2_[2] - crop_rect2_[0] + 1, output2.width_);
  EXPECT_EQ(crop_rect2_[3] - crop_rect2_[1] + 1, output2.height_);
  ret = output1.Valid();
  EXPECT_EQ(true, ret);
  ret = output2.Valid();
  EXPECT_EQ(true, ret);

  ImageConvertor convertor;
  ret = convertor.Convert(output1, IMAGE_TOOLS_RAW_YUV_I420, output1_i420);
  EXPECT_EQ(true, ret);
  ret = convertor.Convert(output2, IMAGE_TOOLS_RAW_YUV_I420, output2_i420);
  EXPECT_EQ(true, ret);
  SaveFile("cropper_bgr_1_592x372.i420",
           output1_i420.data_[0],
           output1_i420.data_size_[0]);
  SaveFile("cropper_bgr_2_592x372.i420",
           output2_i420.data_[0],
           output2_i420.data_size_[0]);
  HobotXStreamFreeImage(output1.data_[0]);
  HobotXStreamFreeImage(output2.data_[0]);
  HobotXStreamFreeImage(output1_i420.data_[0]);
  HobotXStreamFreeImage(output2_i420.data_[0]);
}

TEST_F(CropperTest, RGB_Normal) {
  ImageCropper cropper;
  bool ret = true;
  ImageToolsFormatData output1;
  ImageToolsFormatData output2;
  ImageToolsFormatData output1_i420;
  ImageToolsFormatData output2_i420;
  ret = cropper.Crop(rgb_data_,
                     crop_rect1_[0],
                     crop_rect1_[1],
                     crop_rect1_[2],
                     crop_rect1_[3],
                     output1);
  EXPECT_EQ(true, ret);

  ret = cropper.Crop(rgb_data_,
                     crop_rect2_[0],
                     crop_rect2_[1],
                     crop_rect2_[2],
                     crop_rect2_[3],
                     output2);
  EXPECT_EQ(true, ret);

  EXPECT_EQ(crop_rect1_[2] - crop_rect1_[0] + 1, output1.width_);
  EXPECT_EQ(crop_rect1_[3] - crop_rect1_[1] + 1, output1.height_);
  EXPECT_EQ(crop_rect2_[2] - crop_rect2_[0] + 1, output2.width_);
  EXPECT_EQ(crop_rect2_[3] - crop_rect2_[1] + 1, output2.height_);
  ret = output1.Valid();
  EXPECT_EQ(true, ret);
  ret = output2.Valid();
  EXPECT_EQ(true, ret);

  ImageConvertor convertor;
  ret = convertor.Convert(output1, IMAGE_TOOLS_RAW_YUV_I420, output1_i420);
  EXPECT_EQ(true, ret);
  ret = convertor.Convert(output2, IMAGE_TOOLS_RAW_YUV_I420, output2_i420);
  EXPECT_EQ(true, ret);
  SaveFile("cropper_rgb_1_592x372.i420",
           output1_i420.data_[0],
           output1_i420.data_size_[0]);
  SaveFile("cropper_rgb_2_592x372.i420",
           output2_i420.data_[0],
           output2_i420.data_size_[0]);
  HobotXStreamFreeImage(output1.data_[0]);
  HobotXStreamFreeImage(output2.data_[0]);
  HobotXStreamFreeImage(output1_i420.data_[0]);
  HobotXStreamFreeImage(output2_i420.data_[0]);
}

TEST_F(CropperTest, Gray_Normal) {
  ImageCropper cropper;
  bool ret = true;
  ImageToolsFormatData output1;
  ImageToolsFormatData output2;
  ret = cropper.Crop(gray_data_,
                     crop_rect1_[0],
                     crop_rect1_[1],
                     crop_rect1_[2],
                     crop_rect1_[3],
                     output1);
  EXPECT_EQ(true, ret);

  ret = cropper.Crop(gray_data_,
                     crop_rect2_[0],
                     crop_rect2_[1],
                     crop_rect2_[2],
                     crop_rect2_[3],
                     output2);
  EXPECT_EQ(true, ret);

  EXPECT_EQ(crop_rect1_[2] - crop_rect1_[0] + 1, output1.width_);
  EXPECT_EQ(crop_rect1_[3] - crop_rect1_[1] + 1, output1.height_);
  EXPECT_EQ(crop_rect2_[2] - crop_rect2_[0] + 1, output2.width_);
  EXPECT_EQ(crop_rect2_[3] - crop_rect2_[1] + 1, output2.height_);
  ret = output1.Valid();
  EXPECT_EQ(true, ret);
  ret = output2.Valid();
  EXPECT_EQ(true, ret);

  SaveFile("cropper_gray_1_592x372.gray",
           output1.data_[0],
           output1.data_size_[0]);
  SaveFile("cropper_gray_2_592x372.gray",
           output2.data_[0],
           output2.data_size_[0]);
  HobotXStreamFreeImage(output1.data_[0]);
  HobotXStreamFreeImage(output2.data_[0]);
}

TEST_F(CropperTest, I420_Normal) {
  ImageCropper cropper;
  bool ret = true;
  ImageToolsFormatData output1;
  ImageToolsFormatData output2;
  ret = cropper.Crop(i420_data_,
                     crop_rect1_[0],
                     crop_rect1_[1],
                     crop_rect1_[2],
                     crop_rect1_[3],
                     output1);
  EXPECT_EQ(true, ret);

  ret = cropper.Crop(i420_data_,
                     crop_rect2_[0],
                     crop_rect2_[1],
                     crop_rect2_[2],
                     crop_rect2_[3],
                     output2);
  EXPECT_EQ(true, ret);

  EXPECT_EQ(crop_rect1_[2] - crop_rect1_[0] + 1, output1.width_);
  EXPECT_EQ(crop_rect1_[3] - crop_rect1_[1] + 1, output1.height_);
  EXPECT_EQ(crop_rect2_[2] - crop_rect2_[0] + 1, output2.width_);
  EXPECT_EQ(crop_rect2_[3] - crop_rect2_[1] + 1, output2.height_);
  ret = output1.Valid();
  EXPECT_EQ(true, ret);
  ret = output2.Valid();
  EXPECT_EQ(true, ret);

  SaveFile("cropper_i420_1_592x372.i420",
           output1.data_[0],
           output1.data_size_[0]);
  SaveFile("cropper_i420_2_592x372.i420",
           output2.data_[0],
           output2.data_size_[0]);
  HobotXStreamFreeImage(output1.data_[0]);
  HobotXStreamFreeImage(output2.data_[0]);
}

TEST_F(CropperTest, NV12_Normal) {
  ImageCropper cropper;
  bool ret = true;
  ImageToolsFormatData output1;
  ImageToolsFormatData output2;
  ret = cropper.Crop(nv12_data_,
                     crop_rect1_[0],
                     crop_rect1_[1],
                     crop_rect1_[2],
                     crop_rect1_[3],
                     output1);
  EXPECT_EQ(true, ret);

  ret = cropper.Crop(nv12_data_,
                     crop_rect2_[0],
                     crop_rect2_[1],
                     crop_rect2_[2],
                     crop_rect2_[3],
                     output2);
  EXPECT_EQ(true, ret);

  EXPECT_EQ(crop_rect1_[2] - crop_rect1_[0] + 1, output1.width_);
  EXPECT_EQ(crop_rect1_[3] - crop_rect1_[1] + 1, output1.height_);
  EXPECT_EQ(crop_rect2_[2] - crop_rect2_[0] + 1, output2.width_);
  EXPECT_EQ(crop_rect2_[3] - crop_rect2_[1] + 1, output2.height_);
  ret = output1.Valid();
  EXPECT_EQ(true, ret);
  ret = output2.Valid();
  EXPECT_EQ(true, ret);
  bool uv_same = true;
  uint8_t *output1_uv = output1.data_[0] + output1.width_ * output1.height_;
  uint8_t *output2_uv = output2.data_[0] + output2.width_ * output2.height_;
  for (int i = 0; i < output1.width_ * output1.height_ / 2; ++i) {
    if (output1_uv[i] != output2_uv[i]) {
      uv_same = false;
      break;
    }
  }
  EXPECT_EQ(true, uv_same);

  SaveFile("cropper_nv12_1_592x372.nv12",
           output1.data_[0],
           output1.data_size_[0]);
  SaveFile("cropper_nv12_2_592x372.nv12",
           output2.data_[0],
           output2.data_size_[0]);
  HobotXStreamFreeImage(output1.data_[0]);
  HobotXStreamFreeImage(output2.data_[0]);
}

TEST_F(CropperTest, NV21_Normal) {
  ImageCropper cropper;
  bool ret = true;
  ImageToolsFormatData output1;
  ImageToolsFormatData output2;
  ret = cropper.Crop(nv21_data_,
                     crop_rect1_[0],
                     crop_rect1_[1],
                     crop_rect1_[2],
                     crop_rect1_[3],
                     output1);
  EXPECT_EQ(true, ret);

  ret = cropper.Crop(nv21_data_,
                     crop_rect2_[0],
                     crop_rect2_[1],
                     crop_rect2_[2],
                     crop_rect2_[3],
                     output2);
  EXPECT_EQ(true, ret);

  EXPECT_EQ(crop_rect1_[2] - crop_rect1_[0] + 1, output1.width_);
  EXPECT_EQ(crop_rect1_[3] - crop_rect1_[1] + 1, output1.height_);
  EXPECT_EQ(crop_rect2_[2] - crop_rect2_[0] + 1, output2.width_);
  EXPECT_EQ(crop_rect2_[3] - crop_rect2_[1] + 1, output2.height_);
  ret = output1.Valid();
  EXPECT_EQ(true, ret);
  ret = output2.Valid();
  EXPECT_EQ(true, ret);

  SaveFile("cropper_nv21_1_592x372.nv21",
           output1.data_[0],
           output1.data_size_[0]);
  SaveFile("cropper_nv21_2_592x372.nv21",
           output2.data_[0],
           output2.data_size_[0]);
  HobotXStreamFreeImage(output1.data_[0]);
  HobotXStreamFreeImage(output2.data_[0]);
}

}  //  namespace xstream
