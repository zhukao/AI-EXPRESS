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
#include <string>
#include "gtest/gtest.h"
#include "hobotxstream/image_tools.h"
#include "hobotxstream/imagetools/convertor.h"
#include "hobotxstream/imagetools/decoder.h"
#include "hobotxstream/imagetools/common.h"
#include "include/common.h"
#include "hobotxstream/imagetools/cropper.h"
#include "horizon/vision_type/vision_type.hpp"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

class CropperWithBlackTest : public ::testing::Test {
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
  int crop_rect3_[4] = {10, 560, 600, 930};
  int width_ = 592;
  int height_ = 372;

  int crop_other_[8][4] = {
    {-100, -100, 299, 299},
    {1620, -100, 2019, 299},
    {-100, 780, 299, 1179},
    {1620, 780, 2019, 1179},
    {-99, -99, 299, 299},
    {1620, -99, 2018, 299},
    {-99, 780, 299, 1178},
    {1620, 780, 2018, 1178}
  };
};

TEST_F(CropperWithBlackTest, Bgr_Normal) {
  ImageCropper cropper;
  bool ret = true;
  ImageToolsFormatData output1;
  ImageToolsFormatData output2;
  ImageToolsFormatData output1_i420;
  ImageToolsFormatData output2_i420;
  ret = cropper.CropWithPadBlack(bgr_data_,
                     crop_rect1_[0],
                     crop_rect1_[1],
                     crop_rect1_[2],
                     crop_rect1_[3],
                     output1);
  EXPECT_EQ(true, ret);

  ret = cropper.CropWithPadBlack(bgr_data_,
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
  SaveFile("CropWithPadBlack_bgr_1_592x372.i420",
           output1_i420.data_[0],
           output1_i420.data_size_[0]);
  SaveFile("CropWithPadBlack_bgr_2_592x372.i420",
           output2_i420.data_[0],
           output2_i420.data_size_[0]);
  HobotXStreamFreeImage(output1.data_[0]);
  HobotXStreamFreeImage(output2.data_[0]);
  HobotXStreamFreeImage(output1_i420.data_[0]);
  HobotXStreamFreeImage(output2_i420.data_[0]);
}

TEST_F(CropperWithBlackTest, RGB_Normal) {
  ImageCropper cropper;
  bool ret = true;
  ImageToolsFormatData output1;
  ImageToolsFormatData output2;
  ImageToolsFormatData output1_i420;
  ImageToolsFormatData output2_i420;
  ret = cropper.CropWithPadBlack(rgb_data_,
                     crop_rect1_[0],
                     crop_rect1_[1],
                     crop_rect1_[2],
                     crop_rect1_[3],
                     output1);
  EXPECT_EQ(true, ret);

  ret = cropper.CropWithPadBlack(rgb_data_,
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
  SaveFile("CropWithPadBlack_rgb_1_592x372.i420",
           output1_i420.data_[0],
           output1_i420.data_size_[0]);
  SaveFile("CropWithPadBlack_rgb_2_592x372.i420",
           output2_i420.data_[0],
           output2_i420.data_size_[0]);
  HobotXStreamFreeImage(output1.data_[0]);
  HobotXStreamFreeImage(output2.data_[0]);
  HobotXStreamFreeImage(output1_i420.data_[0]);
  HobotXStreamFreeImage(output2_i420.data_[0]);

  for (int i = 0; i < 4; ++i) {
    ImageToolsFormatData out;
    ImageToolsFormatData out_i420;
    ret = cropper.CropWithPadBlack(rgb_data_,
                     crop_other_[i][0],
                     crop_other_[i][1],
                     crop_other_[i][2],
                     crop_other_[i][3],
                     out);
    EXPECT_EQ(true, ret);
    EXPECT_EQ(out.width_, 400);
    EXPECT_EQ(out.height_, 400);
    ret = convertor.Convert(out, IMAGE_TOOLS_RAW_YUV_I420, out_i420);
    EXPECT_EQ(true, ret);
    std::string file_name = "CropWithPadBlackWithActualPadding_"
                            + std::to_string(i) + "_400x400.i420";
    SaveFile(file_name, out_i420.data_[0], out_i420.data_size_[0]);
    HobotXStreamFreeImage(out.data_[0]);
    HobotXStreamFreeImage(out_i420.data_[0]);
  }
}

TEST_F(CropperWithBlackTest, Gray_Normal) {
  ImageCropper cropper;
  bool ret = true;
  ImageToolsFormatData output1;
  ImageToolsFormatData output2;
  ret = cropper.CropWithPadBlack(gray_data_,
                     crop_rect1_[0],
                     crop_rect1_[1],
                     crop_rect1_[2],
                     crop_rect1_[3],
                     output1);
  EXPECT_EQ(true, ret);

  ret = cropper.CropWithPadBlack(gray_data_,
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

  SaveFile("CropWithPadBlack_gray_1_592x372.gray",
           output1.data_[0],
           output1.data_size_[0]);
  SaveFile("CropWithPadBlack_gray_2_592x372.gray",
           output2.data_[0],
           output2.data_size_[0]);
  HobotXStreamFreeImage(output1.data_[0]);
  HobotXStreamFreeImage(output2.data_[0]);
}

TEST_F(CropperWithBlackTest, I420_Normal) {
  ImageCropper cropper;
  bool ret = true;
  ImageToolsFormatData output1;
  ImageToolsFormatData output2;
  ret = cropper.CropWithPadBlack(i420_data_,
                     crop_rect1_[0],
                     crop_rect1_[1],
                     crop_rect1_[2],
                     crop_rect1_[3],
                     output1);
  EXPECT_EQ(true, ret);

  ret = cropper.CropWithPadBlack(i420_data_,
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

  SaveFile("CropWithPadBlack_i420_1_592x372.i420",
           output1.data_[0],
           output1.data_size_[0]);
  SaveFile("CropWithPadBlack_i420_2_592x372.i420",
           output2.data_[0],
           output2.data_size_[0]);
  HobotXStreamFreeImage(output1.data_[0]);
  HobotXStreamFreeImage(output2.data_[0]);
}

TEST_F(CropperWithBlackTest, NV12_Normal) {
  ImageCropper cropper;
  bool ret = true;
  ImageToolsFormatData output1;
  ImageToolsFormatData output2;
  ImageToolsFormatData output3;
  ret = cropper.CropWithPadBlack(nv12_data_,
                     crop_rect1_[0],
                     crop_rect1_[1],
                     crop_rect1_[2],
                     crop_rect1_[3],
                     output1);
  EXPECT_EQ(true, ret);

  ret = cropper.CropWithPadBlack(nv12_data_,
                     crop_rect2_[0],
                     crop_rect2_[1],
                     crop_rect2_[2],
                     crop_rect2_[3],
                     output2);
  EXPECT_EQ(true, ret);

  ret = cropper.CropWithPadBlack(nv12_data_,
                     crop_rect3_[0],
                     crop_rect3_[1],
                     crop_rect3_[2],
                     crop_rect3_[3],
                     output3);
  EXPECT_EQ(true, ret);

  EXPECT_EQ(crop_rect1_[2] - crop_rect1_[0] + 1, output1.width_);
  EXPECT_EQ(crop_rect1_[3] - crop_rect1_[1] + 1, output1.height_);
  EXPECT_EQ(crop_rect2_[2] - crop_rect2_[0] + 1, output2.width_);
  EXPECT_EQ(crop_rect2_[3] - crop_rect2_[1] + 1, output2.height_);
  int rect3_width = crop_rect3_[2] - crop_rect3_[0] + 1;
  int rect3_height = crop_rect3_[3] - crop_rect3_[1] + 1;
  rect3_width = rect3_width + (rect3_width & (static_cast<int>(0X01)));
  rect3_height = rect3_height + (rect3_height & (static_cast<int>(0X01)));
  EXPECT_EQ(rect3_width, output3.width_);
  EXPECT_EQ(rect3_height, output3.height_);
  ret = output1.Valid();
  EXPECT_EQ(true, ret);
  ret = output2.Valid();
  EXPECT_EQ(true, ret);
  ret = output3.Valid();
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

  SaveFile("CropWithPadBlack_nv12_1_592x372.nv12",
           output1.data_[0],
           output1.data_size_[0]);
  SaveFile("CropWithPadBlack_nv12_2_592x372.nv12",
           output2.data_[0],
           output2.data_size_[0]);
  SaveFile("CropWithPadBlack_nv12_3_592x372.nv12",
           output3.data_[0],
           output3.data_size_[0]);
  HobotXStreamFreeImage(output1.data_[0]);
  HobotXStreamFreeImage(output2.data_[0]);
  HobotXStreamFreeImage(output3.data_[0]);
  // crop with seperate y and uv, nv12 format
  int y_size = nv12_data_.first_stride_ * nv12_data_.height_;
  int uv_size = y_size >> 1;
  uint8_t *y = nullptr;
  HobotXStreamAllocImage(y_size, &y);
  uint8_t *uv = nullptr;
  HobotXStreamAllocImage(uv_size, &uv);
  memcpy(y, nv12_data_.data_[0], y_size);
  memcpy(uv,
         nv12_data_.data_[0] + y_size,
         uv_size);
  for (int i = 0; i < 8; ++i) {
    uint8_t *nv12_output = nullptr;
    int nv12_output_size = 0;
    int nv12_output_width = 0;
    int nv12_output_height = 0;
    int nv12_output_first_stride = 0;
    int nv12_output_second_stride = 0;
    const uint8_t *input_nv12_data[3] = {y, uv, nullptr};
    const int input_nv12_size[3] = {y_size, uv_size, 0};
    int api_ret = HobotXStreamCropYuvImageWithPaddingBlack(\
                                  input_nv12_data,
                                  input_nv12_size,
                                  nv12_data_.width_,
                                  nv12_data_.height_,
                                  nv12_data_.first_stride_,
                                  nv12_data_.second_stride_,
                                  IMAGE_TOOLS_RAW_YUV_NV12,
                                  crop_other_[i][0],
                                  crop_other_[i][1],
                                  crop_other_[i][2],
                                  crop_other_[i][3],
                                  &nv12_output,
                                  &nv12_output_size,
                                  &nv12_output_width,
                                  &nv12_output_height,
                                  &nv12_output_first_stride,
                                  &nv12_output_second_stride);
    EXPECT_EQ(0, api_ret);
    EXPECT_NE(nv12_output, nullptr);
    EXPECT_EQ(nv12_output_size, 400 * 400 * 3 / 2);
    EXPECT_EQ(nv12_output_width, 400);
    EXPECT_EQ(nv12_output_height, 400);
    EXPECT_EQ(nv12_output_first_stride, 400);
    EXPECT_EQ(nv12_output_second_stride, 400);
    std::string file_name = "HobotXStreamCropYuvImageWithPaddingBlack"
                            + std::to_string(i) + "_400x400.nv12";
    SaveFile(file_name, nv12_output, nv12_output_size);
    HobotXStreamFreeImage(nv12_output);
  }
#ifdef X2
  hobot::vision::PymImageFrame *image_frame =
                  new hobot::vision::PymImageFrame();
  image_frame->pym_layer = 0;
  image_frame->img.down_scale[0].width = nv12_data_.width_;
  image_frame->img.down_scale[0].height = nv12_data_.height_;
  image_frame->img.down_scale[0].step = nv12_data_.width_;
  image_frame->img.down_scale[0].y_vaddr = reinterpret_cast<int64_t>(y);
  image_frame->img.down_scale[0].c_vaddr = reinterpret_cast<int64_t>(uv);
  for (int i = 0; i < 8; ++i) {
    uint8_t *nv12_output = nullptr;
    int nv12_output_size = 0;
    int nv12_output_width = 0;
    int nv12_output_height = 0;
    int nv12_output_first_stride = 0;
    int nv12_output_second_stride = 0;
    enum HobotXStreamImageToolsPixelFormat nv12_format = IMAGE_TOOLS_RAW_NONE;
    int api_ret = HobotXStreamCropImageFrameWithPaddingBlack(\
                                  static_cast<void *>(image_frame),
                                  crop_other_[i][0],
                                  crop_other_[i][1],
                                  crop_other_[i][2],
                                  crop_other_[i][3],
                                  &nv12_format,
                                  &nv12_output,
                                  &nv12_output_size,
                                  &nv12_output_width,
                                  &nv12_output_height,
                                  &nv12_output_first_stride,
                                  &nv12_output_second_stride);
    EXPECT_EQ(0, api_ret);
    EXPECT_NE(nv12_output, nullptr);
    EXPECT_EQ(nv12_format, IMAGE_TOOLS_RAW_YUV_NV12);
    EXPECT_EQ(nv12_output_size, 400 * 400 * 3 / 2);
    EXPECT_EQ(nv12_output_width, 400);
    EXPECT_EQ(nv12_output_height, 400);
    EXPECT_EQ(nv12_output_first_stride, 400);
    EXPECT_EQ(nv12_output_second_stride, 400);
    std::string file_name = "HobotXStreamCropImageFrameWithPaddingBlack"
                            + std::to_string(i) + "_400x400.nv12";
    SaveFile(file_name, nv12_output, nv12_output_size);
    HobotXStreamFreeImage(nv12_output);
  }
  delete image_frame;
#else
  hobot::vision::CVImageFrame *image_frame =
                  new hobot::vision::CVImageFrame();
  image_frame->pixel_format = kHorizonVisionPixelFormatRawNV12;
  image_frame->img = cv::Mat(nv12_data_.height_ * 3 / 2,
                             nv12_data_.width_,
                             CV_8UC1);
  memcpy(image_frame->img.data, nv12_data_.data_[0], nv12_data_.data_size_[0]);

  for (int i = 0; i < 8; ++i) {
    uint8_t *nv12_output = nullptr;
    int nv12_output_size = 0;
    int nv12_output_width = 0;
    int nv12_output_height = 0;
    int nv12_output_first_stride = 0;
    int nv12_output_second_stride = 0;
    enum HobotXStreamImageToolsPixelFormat nv12_format = IMAGE_TOOLS_RAW_NONE;
    int api_ret = HobotXStreamCropImageFrameWithPaddingBlack(\
                                  static_cast<void *>(image_frame),
                                  crop_other_[i][0],
                                  crop_other_[i][1],
                                  crop_other_[i][2],
                                  crop_other_[i][3],
                                  &nv12_format,
                                  &nv12_output,
                                  &nv12_output_size,
                                  &nv12_output_width,
                                  &nv12_output_height,
                                  &nv12_output_first_stride,
                                  &nv12_output_second_stride);
    EXPECT_EQ(0, api_ret);
    EXPECT_NE(nv12_output, nullptr);
    EXPECT_EQ(nv12_format, IMAGE_TOOLS_RAW_YUV_NV12);
    EXPECT_EQ(nv12_output_size, 400 * 400 * 3 / 2);
    EXPECT_EQ(nv12_output_width, 400);
    EXPECT_EQ(nv12_output_height, 400);
    EXPECT_EQ(nv12_output_first_stride, 400);
    EXPECT_EQ(nv12_output_second_stride, 400);
    std::string file_name = "HobotXStreamCropImageFrameWithPaddingBlack"
                            + std::to_string(i) + "_400x400.nv12";
    SaveFile(file_name, nv12_output, nv12_output_size);
    HobotXStreamFreeImage(nv12_output);
  }
  delete image_frame;
#endif
  HobotXStreamFreeImage(y);
  HobotXStreamFreeImage(uv);
}


TEST_F(CropperWithBlackTest, NV21_Normal) {
  ImageCropper cropper;
  bool ret = true;
  ImageToolsFormatData output1;
  ImageToolsFormatData output2;
  ret = cropper.CropWithPadBlack(nv21_data_,
                     crop_rect1_[0],
                     crop_rect1_[1],
                     crop_rect1_[2],
                     crop_rect1_[3],
                     output1);
  EXPECT_EQ(true, ret);

  ret = cropper.CropWithPadBlack(nv21_data_,
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

  SaveFile("CropWithPadBlack_nv21_1_592x372.nv21",
           output1.data_[0],
           output1.data_size_[0]);
  SaveFile("CropWithPadBlack_nv21_2_592x372.nv21",
           output2.data_[0],
           output2.data_size_[0]);
  HobotXStreamFreeImage(output1.data_[0]);
  HobotXStreamFreeImage(output2.data_[0]);
}

}  //  namespace xstream
