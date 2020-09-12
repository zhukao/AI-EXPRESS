/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @brief     test class ImageRotater
 * @author    zhengzheng.ge
 * @email     zhengzheng.ge@horizon.ai
 * @version   0.0.0.1
 * @date      2019.11.16
 */

#include <iostream>
#include "gtest/gtest.h"
#include "hobotxstream/imagetools/rotater.h"
#include "hobotxstream/imagetools/convertor.h"
#include "hobotxstream/imagetools/decoder.h"
#include "hobotxstream/imagetools/common.h"
#include "include/common.h"
#include "hobotxstream/image_tools.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

class RotaterTest : public ::testing::Test {
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
    ret = convertor.Convert(i420_data_, IMAGE_TOOLS_RAW_YUV_NV21, nv21_data_);
    HOBOT_CHECK(ret != false);
    ret = convertor.Convert(bgr_data_, IMAGE_TOOLS_RAW_YUV_NV12, nv12_data_);
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
  int src_width_ = 1920;
  int src_height_ = 1080;
};

TEST_F(RotaterTest, Bgr_Normal) {
  ImageRotater rotater;
  bool ret = true;
  ImageToolsFormatData output_90;
  ImageToolsFormatData output_180;
  ImageToolsFormatData output_270;
  ImageToolsFormatData output_i420_90;
  ImageToolsFormatData output_i420_180;
  ImageToolsFormatData output_i420_270;

  ret = rotater.Rotate(bgr_data_,
                       90,
                       output_90);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(src_height_, output_90.width_);
  EXPECT_EQ(src_width_, output_90.height_);

  ret = rotater.Rotate(bgr_data_,
                       180,
                       output_180);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(src_width_, output_180.width_);
  EXPECT_EQ(src_height_, output_180.height_);

  ret = rotater.Rotate(bgr_data_,
                       270,
                       output_270);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(src_height_, output_270.width_);
  EXPECT_EQ(src_width_, output_270.height_);

  ImageConvertor convertor;
  ret = convertor.Convert(output_90, IMAGE_TOOLS_RAW_YUV_I420, output_i420_90);
  EXPECT_EQ(true, ret);
  ret =
    convertor.Convert(output_180, IMAGE_TOOLS_RAW_YUV_I420, output_i420_180);
  EXPECT_EQ(true, ret);
  ret =
    convertor.Convert(output_270, IMAGE_TOOLS_RAW_YUV_I420, output_i420_270);
  EXPECT_EQ(true, ret);
  SaveFile("rotater_bgr_90.i420",
           output_i420_90.data_[0],
           output_i420_90.data_size_[0]);
  SaveFile("rotater_bgr_180.i420",
           output_i420_180.data_[0],
           output_i420_180.data_size_[0]);
  SaveFile("rotater_bgr_270.i420",
           output_i420_270.data_[0],
           output_i420_270.data_size_[0]);
  HobotXStreamFreeImage(output_90.data_[0]);
  HobotXStreamFreeImage(output_180.data_[0]);
  HobotXStreamFreeImage(output_270.data_[0]);
  HobotXStreamFreeImage(output_i420_90.data_[0]);
  HobotXStreamFreeImage(output_i420_180.data_[0]);
  HobotXStreamFreeImage(output_i420_270.data_[0]);
}

TEST_F(RotaterTest, Gray_Normal) {
  ImageRotater rotater;
  bool ret = true;
  ImageToolsFormatData output_90;
  ImageToolsFormatData output_180;
  ImageToolsFormatData output_270;

  ret = rotater.Rotate(gray_data_,
                       90,
                       output_90);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(src_height_, output_90.width_);
  EXPECT_EQ(src_width_, output_90.height_);

  ret = rotater.Rotate(gray_data_,
                       180,
                       output_180);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(src_width_, output_180.width_);
  EXPECT_EQ(src_height_, output_180.height_);

  ret = rotater.Rotate(gray_data_,
                       270,
                       output_270);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(src_height_, output_270.width_);
  EXPECT_EQ(src_width_, output_270.height_);

  SaveFile("rotater_gray_90.gray",
           output_90.data_[0],
           output_90.data_size_[0]);
  SaveFile("rotater_gray_180.gray",
           output_180.data_[0],
           output_180.data_size_[0]);
  SaveFile("rotater_gray_270.gray",
           output_270.data_[0],
           output_270.data_size_[0]);
  HobotXStreamFreeImage(output_90.data_[0]);
  HobotXStreamFreeImage(output_180.data_[0]);
  HobotXStreamFreeImage(output_270.data_[0]);
}

TEST_F(RotaterTest, I420_Normal) {
  ImageRotater rotater;
  bool ret = true;
  ImageToolsFormatData output_90;
  ImageToolsFormatData output_180;
  ImageToolsFormatData output_270;

  ret = rotater.Rotate(i420_data_,
                       90,
                       output_90);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(src_height_, output_90.width_);
  EXPECT_EQ(src_width_, output_90.height_);

  ret = rotater.Rotate(i420_data_,
                       180,
                       output_180);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(src_width_, output_180.width_);
  EXPECT_EQ(src_height_, output_180.height_);

  ret = rotater.Rotate(i420_data_,
                       270,
                       output_270);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(src_height_, output_270.width_);
  EXPECT_EQ(src_width_, output_270.height_);

  SaveFile("rotater_i420_90.i420",
           output_90.data_[0],
           output_90.data_size_[0]);
  SaveFile("rotater_i420_180.i420",
           output_180.data_[0],
           output_180.data_size_[0]);
  SaveFile("rotater_i420_270.i420",
           output_270.data_[0],
           output_270.data_size_[0]);
  HobotXStreamFreeImage(output_90.data_[0]);
  HobotXStreamFreeImage(output_180.data_[0]);
  HobotXStreamFreeImage(output_270.data_[0]);
}

TEST_F(RotaterTest, NV12_Normal) {
  ImageRotater rotater;
  bool ret = true;
  ImageToolsFormatData output_90;
  ImageToolsFormatData output_180;
  ImageToolsFormatData output_270;

  ret = rotater.Rotate(nv12_data_,
                       90,
                       output_90);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(src_height_, output_90.width_);
  EXPECT_EQ(src_width_, output_90.height_);

  ret = rotater.Rotate(nv12_data_,
                       180,
                       output_180);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(src_width_, output_180.width_);
  EXPECT_EQ(src_height_, output_180.height_);

  ret = rotater.Rotate(nv12_data_,
                       270,
                       output_270);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(src_height_, output_270.width_);
  EXPECT_EQ(src_width_, output_270.height_);

  SaveFile("rotater_nv12_90.nv12",
           output_90.data_[0],
           output_90.data_size_[0]);
  SaveFile("rotater_nv12_180.nv12",
           output_180.data_[0],
           output_180.data_size_[0]);
  SaveFile("rotater_nv12_270.nv12",
           output_270.data_[0],
           output_270.data_size_[0]);
  HobotXStreamFreeImage(output_90.data_[0]);
  HobotXStreamFreeImage(output_180.data_[0]);
  HobotXStreamFreeImage(output_270.data_[0]);
}
}  //  namespace xstream
