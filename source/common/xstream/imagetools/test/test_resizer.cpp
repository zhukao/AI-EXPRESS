/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     test class ImageResizer
 * @author    hangjun.yang
 * @email     hangjun.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.14
 */

#include <iostream>
#include "gtest/gtest.h"
#include "hobotxstream/imagetools/convertor.h"
#include "hobotxstream/imagetools/decoder.h"
#include "hobotxstream/imagetools/common.h"
#include "include/common.h"
#include "hobotxstream/image_tools.h"
#include "hobotxstream/imagetools/resizer.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

class ResizerTest : public ::testing::Test {
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
  int dst_width_ = 500;
  int dst_height_ = 500;
};

TEST_F(ResizerTest, Bgr_Normal) {
  ImageResizer resizer;
  bool ret = true;
  ImageToolsFormatData output1;
  ImageToolsFormatData output2;
  ImageToolsFormatData output1_i420;
  ImageToolsFormatData output2_i420;
  ret = resizer.Resize(bgr_data_,
                       dst_width_,
                       dst_height_,
                       false,
                       output1);
  EXPECT_EQ(true, ret);

  ret = resizer.Resize(bgr_data_,
                       dst_width_,
                       dst_height_,
                       true,
                       output2);
  EXPECT_EQ(true, ret);

  EXPECT_EQ(dst_width_, output1.width_);
  EXPECT_EQ(dst_height_, output1.height_);
  EXPECT_EQ(dst_width_, output2.width_);
  EXPECT_EQ(dst_height_, output2.height_);
  ret = output1.Valid();
  EXPECT_EQ(true, ret);
  ret = output2.Valid();
  EXPECT_EQ(true, ret);

  ImageConvertor convertor;
  ret = convertor.Convert(output1, IMAGE_TOOLS_RAW_YUV_I420, output1_i420);
  EXPECT_EQ(true, ret);
  ret = convertor.Convert(output2, IMAGE_TOOLS_RAW_YUV_I420, output2_i420);
  EXPECT_EQ(true, ret);
  SaveFile("resizer_bgr_1_500x500.i420",
           output1_i420.data_[0],
           output1_i420.data_size_[0]);
  SaveFile("resizer_bgr_2_500x500.i420",
           output2_i420.data_[0],
           output2_i420.data_size_[0]);
  HobotXStreamFreeImage(output1.data_[0]);
  HobotXStreamFreeImage(output2.data_[0]);
  HobotXStreamFreeImage(output1_i420.data_[0]);
  HobotXStreamFreeImage(output2_i420.data_[0]);
}

TEST_F(ResizerTest, Gray_Normal) {
  ImageResizer resizer;
  bool ret = true;
  ImageToolsFormatData output1;
  ImageToolsFormatData output2;
  ret = resizer.Resize(gray_data_,
                       dst_width_,
                       dst_height_,
                       false,
                       output1);
  EXPECT_EQ(true, ret);

  ret = resizer.Resize(gray_data_,
                       dst_width_,
                       dst_height_,
                       true,
                       output2);
  EXPECT_EQ(true, ret);

  EXPECT_EQ(dst_width_, output1.width_);
  EXPECT_EQ(dst_height_, output1.height_);
  EXPECT_EQ(dst_width_, output2.width_);
  EXPECT_EQ(dst_height_, output2.height_);
  ret = output1.Valid();
  EXPECT_EQ(true, ret);
  ret = output2.Valid();
  EXPECT_EQ(true, ret);

  SaveFile("resizer_gray_1_500x500.gray",
           output1.data_[0],
           output1.data_size_[0]);
  SaveFile("resizer_gray_2_500x500.gray",
           output2.data_[0],
           output2.data_size_[0]);
  HobotXStreamFreeImage(output1.data_[0]);
  HobotXStreamFreeImage(output2.data_[0]);
}

TEST_F(ResizerTest, I420_Normal) {
  ImageResizer resizer;
  bool ret = true;
  ImageToolsFormatData output1;
  ImageToolsFormatData output2;
  ret = resizer.Resize(i420_data_,
                       dst_width_,
                       dst_height_,
                       false,
                       output1);
  EXPECT_EQ(true, ret);

  ret = resizer.Resize(i420_data_,
                       dst_width_,
                       dst_height_,
                       true,
                       output2);
  EXPECT_EQ(true, ret);

  EXPECT_EQ(dst_width_, output1.width_);
  EXPECT_EQ(dst_height_, output1.height_);
  EXPECT_EQ(dst_width_, output2.width_);
  EXPECT_EQ(dst_height_, output2.height_);
  ret = output1.Valid();
  EXPECT_EQ(true, ret);
  ret = output2.Valid();
  EXPECT_EQ(true, ret);

  SaveFile("resizer_i420_1_500x500.i420",
           output1.data_[0],
           output1.data_size_[0]);
  SaveFile("resizer_i420_2_500x500.i420",
           output2.data_[0],
           output2.data_size_[0]);
  HobotXStreamFreeImage(output1.data_[0]);
  HobotXStreamFreeImage(output2.data_[0]);
}

TEST_F(ResizerTest, NV12_Normal) {
  ImageResizer resizer;
  bool ret = true;
  ImageToolsFormatData output1;
  ImageToolsFormatData output2;
  ret = resizer.Resize(nv12_data_,
                       dst_width_,
                       dst_height_,
                       false,
                       output1);
  EXPECT_EQ(true, ret);

  ret = resizer.Resize(nv12_data_,
                       dst_width_,
                       dst_height_,
                       true,
                       output2);
  EXPECT_EQ(true, ret);

  EXPECT_EQ(dst_width_, output1.width_);
  EXPECT_EQ(dst_height_, output1.height_);
  EXPECT_EQ(dst_width_, output2.width_);
  EXPECT_EQ(dst_height_, output2.height_);
  ret = output1.Valid();
  EXPECT_EQ(true, ret);
  ret = output2.Valid();
  EXPECT_EQ(true, ret);

  SaveFile("resizer_nv12_1_500x500.nv12",
           output1.data_[0],
           output1.data_size_[0]);
  SaveFile("resizer_nv12_2_500x500.nv12",
           output2.data_[0],
           output2.data_size_[0]);
  HobotXStreamFreeImage(output1.data_[0]);
  HobotXStreamFreeImage(output2.data_[0]);
}

TEST_F(ResizerTest, NV21_Normal) {
  ImageResizer resizer;
  bool ret = true;
  ImageToolsFormatData output1;
  ImageToolsFormatData output2;
  ret = resizer.Resize(nv21_data_,
                       dst_width_,
                       dst_height_,
                       false,
                       output1);
  EXPECT_EQ(true, ret);

  ret = resizer.Resize(nv21_data_,
                       dst_width_,
                       dst_height_,
                       true,
                       output2);
  EXPECT_EQ(true, ret);

  EXPECT_EQ(dst_width_, output1.width_);
  EXPECT_EQ(dst_height_, output1.height_);
  EXPECT_EQ(dst_width_, output2.width_);
  EXPECT_EQ(dst_height_, output2.height_);
  ret = output1.Valid();
  EXPECT_EQ(true, ret);
  ret = output2.Valid();
  EXPECT_EQ(true, ret);

  SaveFile("resizer_nv21_1_500x500.nv21",
           output1.data_[0],
           output1.data_size_[0]);
  SaveFile("resizer_nv21_2_500x500.nv21",
           output2.data_[0],
           output2.data_size_[0]);
  HobotXStreamFreeImage(output1.data_[0]);
  HobotXStreamFreeImage(output2.data_[0]);
}

TEST_F(ResizerTest, NV21_Speed) {
  ImageResizer resizer;
  bool ret = true;
  ImageToolsFormatData output[100];
  int64_t start_time = getMilliSecond();
  for (size_t i = 0; i < 100; ++i) {
    ret = resizer.Resize(nv21_data_,
                         128,
                         128,
                         false,
                         output[i]);
    EXPECT_EQ(true, ret);
  }
  int64_t end_time = getMilliSecond();
  std::cout << "resize nv12 from 1920x1080 to 128x128 100 times, use "
            << (end_time - start_time) << " ms" << std::endl;
  for (size_t i = 0; i < 100; ++i) {
    HobotXStreamFreeImage(output[i].data_[0]);
  }
}
}  //  namespace xstream
