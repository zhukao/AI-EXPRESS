/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     test class ImageDecoder
 * @author    hangjun.yang
 * @email     hangjun.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.10
 */

#include <fstream>
#include <iostream>
#include "gtest/gtest.h"
#include "hobotxstream/imagetools/decoder.h"
#include "hobotxstream/imagetools/common.h"
#include "hobotxstream/image_tools.h"
#include "include/common.h"

namespace xstream {

class DecodeTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    std::ifstream istrm;
    istrm.open("test.jpg", std::ios::binary | std::ios::in);
    if (!istrm.good()) {
      input_data_ = nullptr;
      return;
    }
    istrm.seekg(0, std::ios::end);
    input_data_size_ = istrm.tellg();
    istrm.seekg(0, std::ios::beg);
    HobotXStreamAllocImage(input_data_size_, &input_data_);
    istrm.read(reinterpret_cast<char *>(input_data_), input_data_size_);
    istrm.close();
  }
  virtual void TearDown() {
    if (input_data_) {
      HobotXStreamFreeImage(input_data_);
      input_data_ = nullptr;
      input_data_size_ = 0;
    }
  }
  uint8_t *input_data_ = nullptr;
  int input_data_size_ = 0;
};

TEST_F(DecodeTest, Bgr_Normal) {
  ImageDecoder decoder;
  ImageToolsFormatData output;
  bool ret = decoder.Decode(input_data_, input_data_size_,
                            IMAGE_TOOLS_RAW_BGR, output);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(output.width_, 1920);
  EXPECT_EQ(output.height_, 1080);
  EXPECT_EQ(output.first_stride_, 1920 * 3);
  EXPECT_EQ(output.data_size_[0], 1920 * 1080 * 3);
  EXPECT_NE(output.data_[0], nullptr);
  SaveFile("decode_bgr.bgr", output.data_[0], output.data_size_[0]);
  HobotXStreamFreeImage(output.data_[0]);
}

TEST_F(DecodeTest, Rgb_Normal) {
  ImageDecoder decoder;
  ImageToolsFormatData output;
  bool ret = decoder.Decode(input_data_, input_data_size_,
                                 IMAGE_TOOLS_RAW_RGB, output);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(output.width_, 1920);
  EXPECT_EQ(output.height_, 1080);
  EXPECT_EQ(output.first_stride_, 1920 * 3);
  EXPECT_EQ(output.data_size_[0], 1920 * 1080 * 3);
  EXPECT_NE(output.data_[0], nullptr);
  SaveFile("decode_rgb.rgb", output.data_[0], output.data_size_[0]);
  HobotXStreamFreeImage(output.data_[0]);
}

TEST_F(DecodeTest, Gray_Normal) {
  ImageDecoder decoder;
  ImageToolsFormatData output;
  bool ret = decoder.Decode(input_data_, input_data_size_,
                            IMAGE_TOOLS_RAW_GRAY, output);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(output.width_, 1920);
  EXPECT_EQ(output.height_, 1080);
  EXPECT_EQ(output.first_stride_, 1920);
  EXPECT_EQ(output.data_size_[0], 1920 * 1080);
  EXPECT_NE(output.data_[0], nullptr);
  SaveFile("decode_gray.gray", output.data_[0], output.data_size_[0]);
  HobotXStreamFreeImage(output.data_[0]);
}

TEST_F(DecodeTest, I420_Normal) {
  ImageDecoder decoder;
  ImageToolsFormatData output;
  bool ret = decoder.Decode(input_data_, input_data_size_,
                            IMAGE_TOOLS_RAW_YUV_I420, output);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(output.width_, 1920);
  EXPECT_EQ(output.height_, 1080);
  EXPECT_EQ(output.first_stride_, 1920);
  EXPECT_EQ(output.second_stride_, 1920 / 2);
  EXPECT_EQ(output.data_size_[0], 1920 * 1080 * 3 / 2);
  EXPECT_NE(output.data_[0], nullptr);
  SaveFile("decode_i420.i420", output.data_[0], output.data_size_[0]);
  HobotXStreamFreeImage(output.data_[0]);
}

TEST_F(DecodeTest, NV12_Normal) {
  ImageDecoder decoder;
  ImageToolsFormatData output;
  bool ret = decoder.Decode(input_data_, input_data_size_,
                                  IMAGE_TOOLS_RAW_YUV_NV12, output);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(output.width_, 1920);
  EXPECT_EQ(output.height_, 1080);
  EXPECT_EQ(output.first_stride_, 1920);
  EXPECT_EQ(output.second_stride_, 1920);
  EXPECT_EQ(output.data_size_[0], 1920 * 1080 * 3 / 2);
  EXPECT_NE(output.data_[0], nullptr);
  SaveFile("decode_nv12.nv12", output.data_[0], output.data_size_[0]);
  HobotXStreamFreeImage(output.data_[0]);
}

TEST_F(DecodeTest, NV21_Normal) {
  ImageDecoder decoder;
  ImageToolsFormatData output;
  bool ret = decoder.Decode(input_data_, input_data_size_,
                            IMAGE_TOOLS_RAW_YUV_NV21, output);
  EXPECT_EQ(true, ret);
  EXPECT_EQ(output.width_, 1920);
  EXPECT_EQ(output.height_, 1080);
  EXPECT_EQ(output.first_stride_, 1920);
  EXPECT_EQ(output.second_stride_, 1920);
  EXPECT_EQ(output.data_size_[0], 1920 * 1080 * 3 / 2);
  EXPECT_NE(output.data_[0], nullptr);
  SaveFile("decode_nv21.nv21", output.data_[0], output.data_size_[0]);
  HobotXStreamFreeImage(output.data_[0]);
}

}  // namespace xstream
