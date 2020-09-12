/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     test class ImageConvertor
 * @author    hangjun.yang
 * @email     hangjun.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.10
 */

#include <fstream>
#include <iostream>
#include "gtest/gtest.h"
#include "hobotxstream/imagetools/convertor.h"
#include "hobotxstream/imagetools/decoder.h"
#include "hobotxstream/imagetools/common.h"
#include "include/common.h"
#include "hobotxstream/image_tools.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

class ConvertFromBgrTest : public ::testing::Test {
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
                              none_stride_input_);
    HOBOT_CHECK(ret != false);
    HobotXStreamFreeImage(jpg_data);

    uint8_t *stride_output = nullptr;
    HobotXStreamAllocImage(stride_ * height_, &stride_output);
    uint8_t *output_pos = stride_output;
    uint8_t *input_pos = none_stride_input_.data_[0];
    int input_stride = none_stride_input_.first_stride_;
    for (int i = 0; i < height_; ++i) {
      memcpy(output_pos, input_pos, width_ * 3);
      output_pos += stride_;
      input_pos += input_stride;
    }
    stride_input_.data_[0] = stride_output;
    stride_input_.data_size_[0] = stride_ * height_;
    stride_input_.width_ = width_;
    stride_input_.height_ = height_;
    stride_input_.first_stride_ = stride_;
    stride_input_.format_ = IMAGE_TOOLS_RAW_BGR;
  }

  virtual void TearDown() {
    HobotXStreamFreeImage(none_stride_input_.data_[0]);
    HobotXStreamFreeImage(stride_input_.data_[0]);
  }

  ImageToolsFormatData none_stride_input_;
  ImageToolsFormatData stride_input_;
  int stride_ = 1920 * 3 + 100;
  int width_ = 1920;
  int height_ = 1080;
};

TEST_F(ConvertFromBgrTest, BgrToRgb_Normal) {
  ImageConvertor convertor;
  ImageToolsFormatData output;
  ImageToolsFormatData stride_output;
  bool ret = convertor.Convert(none_stride_input_,
                               IMAGE_TOOLS_RAW_RGB,
                               output);
  EXPECT_EQ(ret, true);
  ret = true;
  ret = CheckOutput(output, width_, height_);
  EXPECT_EQ(ret, true);

  ret = convertor.Convert(stride_input_,
                          IMAGE_TOOLS_RAW_RGB,
                          stride_output);
  EXPECT_EQ(ret, true);
  ret = true;
  ret = CheckOutput(stride_output, width_, height_);
  EXPECT_EQ(ret, true);
  EXPECT_EQ(output.first_stride_, stride_output.first_stride_);
  uint8_t *output_data = output.data_[0];
  uint8_t *stride_output_data = stride_output.data_[0];
  ret = true;
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < stride_output.first_stride_; ++x) {
      if (output_data[x] != stride_output_data[x]) {
        ret = false;
        break;
      }
    }
    if (!ret) {
      break;
    }
    output_data += stride_output.first_stride_;
    stride_output_data += stride_output.first_stride_;
  }

  // test input bgr == output rgb
  uint8_t *input_data = none_stride_input_.data_[0];
  int input_stride = none_stride_input_.first_stride_;
  output_data = output.data_[0];
  ret = true;
  for (int y = 0; y < height_; ++y) {
    input_data = none_stride_input_.data_[0] + y * input_stride;
    output_data = output.data_[0] + y * output.first_stride_;
    for (int x = 0; x < width_; x+=3) {
      if (output_data[0] != input_data[2]
          || output_data[1] != input_data[1]
          || output_data[2] != input_data[0]) {
        ret = false;
        break;
      }
      input_data +=3;
      output_data +=3;
    }
    if (!ret) {
      break;
    }
  }

  EXPECT_EQ(ret, true);
  HobotXStreamFreeImage(output.data_[0]);
  HobotXStreamFreeImage(stride_output.data_[0]);
}

TEST_F(ConvertFromBgrTest, BgrToGray_Normal) {
  ImageConvertor convertor;
  ImageToolsFormatData output;
  ImageToolsFormatData stride_output;
  bool ret = convertor.Convert(none_stride_input_,
                               IMAGE_TOOLS_RAW_GRAY,
                               output);
  EXPECT_EQ(ret, true);
  ret = true;
  ret = CheckOutput(output, width_, height_);
  EXPECT_EQ(ret, true);

  ret = convertor.Convert(stride_input_,
                          IMAGE_TOOLS_RAW_GRAY,
                          stride_output);
  EXPECT_EQ(ret, true);
  ret = true;
  ret = CheckOutput(stride_output, width_, height_);
  EXPECT_EQ(ret, true);

  uint8_t *output_data = output.data_[0];
  uint8_t *stride_output_data = stride_output.data_[0];
  ret = true;
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      if (output_data[x] != stride_output_data[x]) {
        ret = false;
        break;
      }
    }
    if (!ret) {
      break;
    }
    output_data += width_;
    stride_output_data += width_;
  }
  SaveFile("convert_bgr_gray.gray", output.data_[0], output.data_size_[0]);
  HobotXStreamFreeImage(output.data_[0]);
  HobotXStreamFreeImage(stride_output.data_[0]);
}

TEST_F(ConvertFromBgrTest, BgrToI420_Normal) {
  ImageConvertor convertor;
  ImageToolsFormatData output;
  ImageToolsFormatData stride_output;
  bool ret = convertor.Convert(none_stride_input_,
                               IMAGE_TOOLS_RAW_YUV_I420,
                               output);
  EXPECT_EQ(ret, true);
  ret = CheckOutput(output, width_, height_);
  EXPECT_EQ(ret, true);

  ret = convertor.Convert(stride_input_,
                          IMAGE_TOOLS_RAW_YUV_I420,
                          stride_output);
  EXPECT_EQ(ret, true);
  ret = CheckOutput(stride_output, width_, height_);
  EXPECT_EQ(ret, true);

  uint8_t *output_data = output.data_[0];
  uint8_t *stride_output_data = stride_output.data_[0];
  ret = true;
  for (int y = 0; y < height_ * 3 / 2; ++y) {
    for (int x = 0; x < width_; ++x) {
      if (output_data[x] != stride_output_data[x]) {
        ret = false;
        break;
      }
    }
    output_data += width_;
    stride_output_data += width_;
    if (!ret) {
        break;
    }
  }
  EXPECT_EQ(ret, true);
  SaveFile("convert_bgr_i420.i420", output.data_[0], output.data_size_[0]);

  HobotXStreamFreeImage(output.data_[0]);
  HobotXStreamFreeImage(stride_output.data_[0]);
}

// TEST_F(ConvertFromBgrTest, BgrToNv12_Normal) {

// }

// TEST_F(ConvertFromBgrTest, BgrToNv21_Normal) {

// }

}  // namespace xstream

