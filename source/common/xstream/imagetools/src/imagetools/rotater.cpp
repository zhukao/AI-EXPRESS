/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @brief     provides xstream image Rotater
 * @author    zhengzheng.ge
 * @email     zhengzheng.ge@horizon.ai
 * @version   0.0.0.1
 * @date      2019.11.15
 */

#include "hobotxstream/imagetools/rotater.h"
#include <string.h>
#include <libyuv.h>
#include <iostream>
#include <set>

namespace xstream {
const std::set<int> ImageRotater::degree_set_ = {90, 180, 270};
bool ImageRotater::Rotate(const ImageToolsFormatData &input,
                          const int degree,
                          ImageToolsFormatData &output) {
  if (!input.Valid() || (degree_set_.find(degree) == degree_set_.end())) {
    return false;
  }
  degree_ = degree;
  GetInputImageInfo(input);

  switch (degree_) {
    case 90:
    case 270: {
      output_width_ = input_height_;
      output_height_ = input_width_;
      break;
    }
    case 180: {
      output_width_ = input_width_;
      output_height_ = input_height_;
      break;
    }
  }

  bool ret = false;
  if (IMAGE_TOOLS_RAW_YUV_I420 == input_fmt_) {
    ret = RotateI420();
  } else if (IMAGE_TOOLS_RAW_RGB == input_fmt_
             || IMAGE_TOOLS_RAW_BGR == input_fmt_) {
    ret = RotateRGB();
  } else if (IMAGE_TOOLS_RAW_YUV_NV12 == input_fmt_
             || IMAGE_TOOLS_RAW_YUV_NV21 == input_fmt_) {
    ret = RotateNV();
  } else if (IMAGE_TOOLS_RAW_GRAY == input_fmt_) {
    ret = RotateGray();
  } else {
    ret = false;
  }
  if (!ret) {
    return false;
  }
  output_fmt_ = input_fmt_;

  SetOutputImageInfo(output);
  return true;
}

bool ImageRotater::RotateI420() {
  output_first_stride_ = output_width_;
  output_second_stride_ = output_width_ >> 1;
  output_data_size_ = output_width_ * output_height_ * 3 / 2;
  if (0 != HobotXStreamAllocImage(output_data_size_, &output_data_)) {
    return false;
  }
  const uint8_t *input_y = nullptr;
  const uint8_t *input_u = nullptr;
  const uint8_t *input_v = nullptr;
  if (FotmatDataArrayType::kContinueType == input_array_type_) {
    input_y = input_data_[0];
    input_u = input_y + input_first_stride_ * input_height_;
    input_v = input_u + input_second_stride_ * input_height_ / 2;
  } else {
    input_y = input_data_[0];
    input_u = input_data_[1];
    input_v = input_data_[2];
  }

  uint8_t *output_y = output_data_;
  uint8_t *output_u = output_y + output_first_stride_ * output_height_;
  uint8_t *output_v = output_u + output_second_stride_ * output_height_ / 2;

  // rotate
  int ret = libyuv::I420Rotate(input_y,
                               input_first_stride_,
                               input_u,
                               input_second_stride_,
                               input_v,
                               input_second_stride_,
                               output_y,
                               output_first_stride_,
                               output_u,
                               output_second_stride_,
                               output_v,
                               output_second_stride_,
                               input_width_,
                               input_height_,
                               (libyuv::RotationMode)degree_);
  if (ret) {
    HobotXStreamFreeImage(output_data_);
    output_data_ = nullptr;
    return false;
  }

  return true;
}

bool ImageRotater::RotateNV() {
  output_first_stride_ = output_width_;
  output_second_stride_ = output_width_;
  output_data_size_ = output_width_ * output_height_ * 3 / 2;
  if (0 != HobotXStreamAllocImage(output_data_size_, &output_data_)) {
    return false;
  }

  const uint8_t *input_y = nullptr;
  const uint8_t *input_uv = nullptr;
  if (FotmatDataArrayType::kContinueType == input_array_type_) {
    input_y = input_data_[0];
    input_uv = input_y + input_first_stride_ * input_height_;
  } else {
    input_y = input_data_[0];
    input_uv = input_data_[1];
  }

  uint8_t *tmp_uv_buf = nullptr;
  int tmp_uv_size = output_second_stride_ * output_height_ / 2;
  if (0 != HobotXStreamAllocImage(tmp_uv_size, &tmp_uv_buf)) {
    HobotXStreamFreeImage(output_data_);
    return false;
  }

  uint8_t *output_y = output_data_;
  uint8_t *output_uv = output_y + output_first_stride_ * output_height_;

  // rotate to i420
  int ret = libyuv::NV12ToI420Rotate(input_y,
                                     input_first_stride_,
                                     input_uv,
                                     input_second_stride_,
                                     output_y,
                                     output_first_stride_,
                                     tmp_uv_buf,
                                     output_second_stride_ / 2,
                                     tmp_uv_buf + tmp_uv_size / 2,
                                     output_second_stride_ / 2,
                                     input_width_,
                                     input_height_,
                                     (libyuv::RotationMode)degree_);
  if (ret) {
    HobotXStreamFreeImage(output_data_);
    HobotXStreamFreeImage(tmp_uv_buf);
    output_data_ = nullptr;
    return false;
  }
  // merge u v
  uint8_t *dst_uv = output_uv;
  uint8_t *src_u = &tmp_uv_buf[0];
  uint8_t *src_v = &tmp_uv_buf[tmp_uv_size / 2];
  int stride_u = output_second_stride_ / 2,
      stride_v = output_second_stride_ / 2;
  int halfwidth = output_width_ >> 1;
  int halfheight = output_height_ >> 1;
  for (int y = 0; y < halfheight; ++y) {
    uint8_t *next_uv = dst_uv + output_second_stride_;
    uint8_t *next_u = src_u + stride_u;
    uint8_t *next_v = src_v + stride_v;
    for (int x = 0; x < halfwidth; ++x) {
      *dst_uv++ = *src_u++;
      *dst_uv++ = *src_v++;
    }
    dst_uv = next_uv;
    src_u = next_u;
    src_v = next_v;
  }

  HobotXStreamFreeImage(tmp_uv_buf);
  return true;
}

bool ImageRotater::RotateRGB() {
  output_first_stride_ = output_width_ * 3;
  output_data_size_ = output_first_stride_ * output_height_;
  if (0 != HobotXStreamAllocImage(output_data_size_, &output_data_)) {
    return false;
  }
  uint8_t *tmp_src_bufs[3] = {nullptr};
  uint8_t *tmp_dst_bufs[3] = {nullptr};
  uint32_t planar_src_size = input_width_ * input_height_;
  uint32_t planar_dst_size = output_width_ * output_height_;
  bool alloc_ret = true;
  for (int planar_idx = 0; planar_idx < 3; planar_idx++) {
    if (HobotXStreamAllocImage(planar_src_size, &tmp_src_bufs[planar_idx]) ||
        HobotXStreamAllocImage(planar_dst_size, &tmp_dst_bufs[planar_idx])) {
      alloc_ret = false;
      break;
    }
  }
  if (!alloc_ret) {
    for (int planar_idx = 0; planar_idx < 3; planar_idx++) {
      HobotXStreamFreeImage(tmp_src_bufs[planar_idx]);
      HobotXStreamFreeImage(tmp_dst_bufs[planar_idx]);
    }
    HobotXStreamFreeImage(output_data_);
    return false;
  }

  // split r g b
  for (int rgb_idx = 0; rgb_idx < input_data_size_[0]; rgb_idx += 3) {
    uint32_t planar_idx = rgb_idx / 3;
    for (int inner_idx = 0; inner_idx < 3; inner_idx++) {
      tmp_src_bufs[inner_idx][planar_idx] = input_data_[0][rgb_idx + inner_idx];
    }
  }

  bool ret = true;
  for (int inner_idx = 0; inner_idx < 3; inner_idx++) {
    if (libyuv::RotatePlane(tmp_src_bufs[inner_idx], input_width_,
                            tmp_dst_bufs[inner_idx], output_width_,
                            input_width_, input_height_,
                            (libyuv::RotationMode)degree_)) {
      ret = false;
      break;
    }
  }
  if (!ret) {
    for (int planar_idx = 0; planar_idx < 3; planar_idx++) {
      HobotXStreamFreeImage(tmp_src_bufs[planar_idx]);
      HobotXStreamFreeImage(tmp_dst_bufs[planar_idx]);
    }
    HobotXStreamFreeImage(output_data_);
    return false;
  }
  // merge r g b
  for (uint idx = 0; idx < planar_dst_size; idx++) {
    for (int inner_idx = 0; inner_idx < 3; inner_idx++) {
      output_data_[idx * 3 + inner_idx] = tmp_dst_bufs[inner_idx][idx];
    }
  }

  for (int planar_idx = 0; planar_idx < 3; planar_idx++) {
    HobotXStreamFreeImage(tmp_src_bufs[planar_idx]);
    HobotXStreamFreeImage(tmp_dst_bufs[planar_idx]);
  }

  return true;
}

bool ImageRotater::RotateGray() {
  output_first_stride_ = output_width_;
  output_data_size_ = output_first_stride_ * output_height_;
  if (0 != HobotXStreamAllocImage(output_data_size_, &output_data_)) {
    return false;
  }

  bool ret = libyuv::RotatePlane(input_data_[0], input_first_stride_,
                                 output_data_, output_first_stride_,
                                 input_width_, input_height_,
                                 (libyuv::RotationMode)degree_);
  if (ret) {
    HobotXStreamFreeImage(output_data_);
    output_data_ = nullptr;
    return false;
  }

  return true;
}

}  // namespace xstream
