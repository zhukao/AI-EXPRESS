/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     test class ImagePadding
 * @author    hangjun.yang
 * @email     hangjun.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.15
 */

#include <fstream>
#include <iostream>
#include "gtest/gtest.h"
#include "hobotxstream/imagetools/convertor.h"
#include "hobotxstream/imagetools/decoder.h"
#include "hobotxstream/imagetools/common.h"
#include "include/common.h"
#include "hobotxstream/image_tools.h"
#include "hobotxstream/imagetools/padding.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

class PaddingTest : public ::testing::Test {
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
    ret = convertor.Convert(rgb_data_, IMAGE_TOOLS_RAW_YUV_NV12, nv12_data_);
    HOBOT_CHECK(ret != false);
    ret = convertor.Convert(nv12_data_, IMAGE_TOOLS_RAW_YUV_NV21, nv21_data_);
    HOBOT_CHECK(ret != false);
    ret = convertor.Convert(bgr_data_, IMAGE_TOOLS_RAW_GRAY, gray_data_);
    HOBOT_CHECK(ret != false);
    ret = convertor.Convert(bgr_data_, IMAGE_TOOLS_RAW_YUV_I420, i420_data_);
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
  int padding_left_ = 100;
  int padding_right_ = 200;
  int padding_top_ = 200;
  int padding_bottom_ = 400;
  uint8_t gray_value_[3] = {0, 0, 0};
  uint8_t rgb_value_[3] = {0, 0, 0};
  uint8_t yuv_value_[3] = {0, 128, 128};
};

TEST_F(PaddingTest, Bgr_Normal) {
  ImagePadding padding;
  bool ret = true;
  ImageToolsFormatData output;
  ImageToolsFormatData output_i420;
  ret = padding.Pad(bgr_data_,
                    padding_left_,
                    padding_right_,
                    padding_top_,
                    padding_bottom_,
                    rgb_value_,
                    output);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(1920 + padding_left_ + padding_right_, output.width_);
  EXPECT_EQ(1080 + padding_top_ + padding_bottom_, output.height_);
  ret = output.Valid();
  EXPECT_EQ(true, ret);

  ImageConvertor convertor;
  ret = convertor.Convert(output, IMAGE_TOOLS_RAW_YUV_I420, output_i420);
  EXPECT_EQ(true, ret);
  SaveFile("padding_bgr_2220x1680.i420",
           output_i420.data_[0],
           output_i420.data_size_[0]);
  HobotXStreamFreeImage(output.data_[0]);
  HobotXStreamFreeImage(output_i420.data_[0]);
}

TEST_F(PaddingTest, Rgb_Normal) {
  ImagePadding padding;
  bool ret = true;
  ImageToolsFormatData output;
  ImageToolsFormatData output_i420;
  ret = padding.Pad(rgb_data_,
                    padding_left_,
                    padding_right_,
                    padding_top_,
                    padding_bottom_,
                    rgb_value_,
                    output);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(1920 + padding_left_ + padding_right_, output.width_);
  EXPECT_EQ(1080 + padding_top_ + padding_bottom_, output.height_);
  ret = output.Valid();
  EXPECT_EQ(true, ret);

  ImageConvertor convertor;
  ret = convertor.Convert(output, IMAGE_TOOLS_RAW_YUV_I420, output_i420);
  EXPECT_EQ(true, ret);
  SaveFile("padding_rgb_2220x1680.i420",
           output_i420.data_[0],
           output_i420.data_size_[0]);
  HobotXStreamFreeImage(output.data_[0]);
  HobotXStreamFreeImage(output_i420.data_[0]);
}

TEST_F(PaddingTest, Rgb_Normal2) {
  ImagePadding padding;
  bool ret = true;
  ImageToolsFormatData output;
  ImageToolsFormatData output_i420;
  ret = padding.Pad(rgb_data_,
                    padding_left_,
                    padding_right_,
                    padding_top_,
                    padding_bottom_,
                    rgb_value_,
                    output);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(1920 + padding_left_ + padding_right_, output.width_);
  EXPECT_EQ(1080 + padding_top_ + padding_bottom_, output.height_);
  ret = output.Valid();
  EXPECT_EQ(true, ret);

  ImageConvertor convertor;
  ret = convertor.Convert(output, IMAGE_TOOLS_RAW_YUV_I420, output_i420);
  EXPECT_EQ(true, ret);
  SaveFile("padding_rgb_2220x1680.i420",
           output_i420.data_[0],
           output_i420.data_size_[0]);
  HobotXStreamFreeImage(output.data_[0]);
  HobotXStreamFreeImage(output_i420.data_[0]);
}

TEST_F(PaddingTest, Gray_Normal) {
  ImagePadding padding;
  bool ret = true;
  ImageToolsFormatData output;
  ret = padding.Pad(gray_data_,
                    padding_left_,
                    padding_right_,
                    padding_top_,
                    padding_bottom_,
                    gray_value_,
                    output);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(1920 + padding_left_ + padding_right_, output.width_);
  EXPECT_EQ(1080 + padding_top_ + padding_bottom_, output.height_);
  ret = output.Valid();
  EXPECT_EQ(true, ret);

  SaveFile("padding_gray_2220x1680.gray",
           output.data_[0],
           output.data_size_[0]);
  HobotXStreamFreeImage(output.data_[0]);
}

TEST_F(PaddingTest, I420_Normal) {
  ImagePadding padding;
  bool ret = true;
  ImageToolsFormatData output;
  ret = padding.Pad(i420_data_,
                    padding_left_,
                    padding_right_,
                    padding_top_,
                    padding_bottom_,
                    yuv_value_,
                    output);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(1920 + padding_left_ + padding_right_, output.width_);
  EXPECT_EQ(1080 + padding_top_ + padding_bottom_, output.height_);
  ret = output.Valid();
  EXPECT_EQ(true, ret);

  SaveFile("padding_i420_2220x1680.i420",
           output.data_[0],
           output.data_size_[0]);
  HobotXStreamFreeImage(output.data_[0]);
}

TEST_F(PaddingTest, NV12_Normal) {
  ImagePadding padding;
  bool ret = true;
  ImageToolsFormatData output;
  ret = padding.Pad(nv12_data_,
                    padding_left_,
                    padding_right_,
                    padding_top_,
                    padding_bottom_,
                    yuv_value_,
                    output);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(1920 + padding_left_ + padding_right_, output.width_);
  EXPECT_EQ(1080 + padding_top_ + padding_bottom_, output.height_);
  ret = output.Valid();
  EXPECT_EQ(true, ret);

  SaveFile("padding_nv12_2220x1680.nv12",
           output.data_[0],
           output.data_size_[0]);
  HobotXStreamFreeImage(output.data_[0]);
}

TEST_F(PaddingTest, NV21_Normal) {
  ImagePadding padding;
  bool ret = true;
  ImageToolsFormatData output;
  ret = padding.Pad(nv21_data_,
                    padding_left_,
                    padding_right_,
                    padding_top_,
                    padding_bottom_,
                    yuv_value_,
                    output);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(1920 + padding_left_ + padding_right_, output.width_);
  EXPECT_EQ(1080 + padding_top_ + padding_bottom_, output.height_);
  ret = output.Valid();
  EXPECT_EQ(true, ret);

  SaveFile("padding_nv21_2220x1680.nv21",
           output.data_[0],
           output.data_size_[0]);
  HobotXStreamFreeImage(output.data_[0]);
}

}  //  namespace xstream
