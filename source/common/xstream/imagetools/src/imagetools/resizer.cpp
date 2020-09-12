/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream image Resizer
 * @author    hangjun.yang
 * @email     hangjun.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.12
 */

#include "hobotxstream/imagetools/resizer.h"
#include <string.h>
#include <libyuv.h>
#include <iostream>
#include "hobotxstream/imagetools/convertor.h"

namespace xstream {

bool ImageResizer::Resize(const ImageToolsFormatData &input,
                          const int dst_width,
                          const int dst_height,
                          const bool fix_aspect_ratio,
                          ImageToolsFormatData &output) {
  if (!input.Valid()) {
    return false;
  }
  if (dst_width <= 0 || dst_height <= 0) {
    return false;
  }
  if (dst_width == input.width_ && dst_height == input.height_) {
    return false;
  }
  GetInputImageInfo(input);

  output_fmt_ = input.format_;
  output_width_ = dst_width;
  output_height_ = dst_height;

  resize_info_.src_width_ = input.width_;
  resize_info_.src_height_ = input.height_;
  resize_info_.dst_width_ = dst_width;
  resize_info_.dst_height_ = dst_height;
  resize_info_.fix_aspect_ratio_ = fix_aspect_ratio;
  if (!CalcResizeInfo()) {
    return false;
  }
  bool ret = false;
  if (IMAGE_TOOLS_RAW_YUV_I420 == input_fmt_) {
    ret = ResizeI420();
  } else if (IMAGE_TOOLS_RAW_RGB == input_fmt_
             || IMAGE_TOOLS_RAW_BGR == input_fmt_
             || IMAGE_TOOLS_RAW_GRAY == input_fmt_) {
    ret = ResizeOnePlane();
  } else if (IMAGE_TOOLS_RAW_YUV_NV12 == input_fmt_
             || IMAGE_TOOLS_RAW_YUV_NV21 == input_fmt_) {
    ret = ResizeNV();
  } else {
    ret = false;
  }
  if (!ret) {
    return false;
  } else {
    SetOutputImageInfo(output);
    return true;
  }
}

bool ImageResizer::ResizeOnePlane() {
  int element_size = 3;
  if (IMAGE_TOOLS_RAW_GRAY == input_fmt_) {
    element_size = 1;
    output_first_stride_ = output_width_;
  } else {
    element_size = 3;
    output_first_stride_ = output_width_ * 3;
  }
  output_data_size_ = output_first_stride_ * output_height_;
  if (0 != HobotXStreamAllocImage(output_data_size_, &output_data_)) {
    return false;
  }
  BilinearInterpolation(resize_info_.src_width_,
                        resize_info_.src_height_,
                        element_size,
                        input_first_stride_,
                        input_data_[0],
                        ratio_dst_width_,
                        ratio_dst_height_,
                        output_first_stride_,
                        output_data_);
  return true;
}

bool ImageResizer::ResizeI420() {
  if ((resize_info_.dst_width_ & static_cast<int>(0X01)) != 0
      || (resize_info_.dst_height_ & static_cast<int>(0X01)) != 0) {
    return false;
  }
  int src_width = resize_info_.src_width_;
  int src_height = resize_info_.src_height_;
  int src_y_stride = input_first_stride_;
  int src_uv_stride = input_second_stride_;

  const uint8_t *src_y = nullptr;
  const uint8_t *src_u = nullptr;
  const uint8_t *src_v = nullptr;
  if (FotmatDataArrayType::kContinueType == input_array_type_) {
    src_y = input_data_[0];
    src_u = src_y + src_y_stride * src_height;
    src_v = src_u + src_uv_stride * src_height / 2;
  } else {
    src_y = input_data_[0];
    src_u = input_data_[1];
    src_v = input_data_[2];
  }

  output_first_stride_ = resize_info_.dst_width_;
  output_second_stride_ = resize_info_.dst_width_ >> 1;
  output_data_size_ = resize_info_.dst_width_ *\
               resize_info_.dst_height_ * 3 / 2;
  if (0 != HobotXStreamAllocImage(output_data_size_, &output_data_)) {
    return false;
  }
  uint8_t *output_y = output_data_;
  uint8_t *output_u = output_y + \
            output_first_stride_ * output_height_;
  uint8_t *output_v = output_u + \
            output_second_stride_ * output_height_ / 2;
  if (libyuv::I420Scale(src_y, src_y_stride,
                        src_u, src_uv_stride,
                        src_v, src_uv_stride,
                        src_width, src_height,
                        output_y, output_first_stride_,
                        output_u, output_second_stride_,
                        output_v, output_second_stride_,
                        ratio_dst_width_,
                        ratio_dst_height_,  // not set error
                        libyuv::kFilterBilinear) != 0) {
    HobotXStreamFreeImage(output_data_);
    output_data_ = nullptr;
    return false;
  }
  if (resize_info_.padding_right_ != 0) {
    uint8_t *output_padding_y = output_y + ratio_dst_width_;
    uint8_t *output_padding_u = output_u + ratio_dst_width_ / 2;
    uint8_t *output_padding_v = output_v + ratio_dst_width_ / 2;
    for (int i = 0; i < ratio_dst_height_; ++i) {
      memset(output_padding_y, 0, resize_info_.padding_right_);
      output_padding_y += output_first_stride_;
      if ((i & 1) == 0) {
        memset(output_padding_u, 128, resize_info_.padding_right_ >> 1);
        memset(output_padding_v, 128, resize_info_.padding_right_ >> 1);
        output_padding_u += output_second_stride_;
        output_padding_v += output_second_stride_;
      }
    }
  }
  if (resize_info_.padding_bottom_ != 0) {
    uint8_t *output_padding_y = output_y + \
                  output_first_stride_ * ratio_dst_height_;
    uint8_t *output_padding_u = output_u + \
                  output_second_stride_ * ratio_dst_height_ / 2;
    uint8_t *output_padding_v = output_v + \
                  output_second_stride_ * ratio_dst_height_ / 2;
    memset(output_padding_y,
           0,
           output_first_stride_ * resize_info_.padding_bottom_);
    memset(output_padding_u,
           128,
           output_second_stride_ * resize_info_.padding_bottom_ / 2);
    memset(output_padding_v,
           128,
           output_second_stride_ * resize_info_.padding_bottom_ / 2);
  }
  return true;
}

bool ImageResizer::ResizeNV() {
  if ((resize_info_.dst_width_ & static_cast<int>(0X01)) != 0
      || (resize_info_.dst_height_ & static_cast<int>(0X01)) != 0) {
    return false;
  }
  int src_width = resize_info_.src_width_;
  int src_height = resize_info_.src_height_;
  int src_y_stride = input_first_stride_;
  int src_uv_stride = input_second_stride_;
  const uint8_t *src_y = nullptr;
  const uint8_t *src_uv = nullptr;
  if (FotmatDataArrayType::kContinueType == input_array_type_) {
    src_y = input_data_[0];
    src_uv = src_y + src_y_stride * src_height;
  } else {
    src_y = input_data_[0];
    src_uv = input_data_[1];
  }

  output_first_stride_ = resize_info_.dst_width_;
  output_second_stride_ = resize_info_.dst_width_;
  output_data_size_ = resize_info_.dst_width_ *\
               resize_info_.dst_height_ * 3 / 2;
  if (0 != HobotXStreamAllocImage(output_data_size_, &output_data_)) {
    return false;
  }
  uint8_t *output_y = output_data_;
  uint8_t *output_uv = output_y + \
            output_first_stride_ * output_height_;
  // split u and v
  uint8_t *pre_uv_buf = nullptr;
  int pre_uv_size = src_uv_stride * src_height / 2;
  if (0 != HobotXStreamAllocImage(pre_uv_size, &pre_uv_buf)) {
    HobotXStreamFreeImage(output_data_);
    return false;
  }
  uint32_t u_v_idx = 0;
  uint32_t u_length = pre_uv_size / 2;
  for (int uv_idx = 0; uv_idx < pre_uv_size; uv_idx += 2) {
    pre_uv_buf[u_v_idx] = src_uv[uv_idx];
    pre_uv_buf[u_v_idx + u_length] = src_uv[uv_idx + 1];
    u_v_idx++;
  }

  uint8_t *resize_uv_buf = nullptr;
  int resize_uv_size = output_second_stride_ * resize_info_.dst_height_ / 2;
  if (0 != HobotXStreamAllocImage(resize_uv_size, &resize_uv_buf)) {
    HobotXStreamFreeImage(output_data_);
    HobotXStreamFreeImage(pre_uv_buf);
    return false;
  }
  // scale Y
  libyuv::ScalePlane(src_y, src_y_stride,
                     src_width, src_height,
                     output_y, output_first_stride_,
                     ratio_dst_width_,
                     ratio_dst_height_,
                     libyuv::kFilterBox);
  // scale uv
  libyuv::ScalePlane(pre_uv_buf, src_uv_stride / 2,
                     src_width / 2, src_height / 2,
                     resize_uv_buf, output_second_stride_ / 2,
                     ratio_dst_width_ / 2,
                     ratio_dst_height_ / 2,
                     libyuv::kFilterBox);
  libyuv::ScalePlane(pre_uv_buf + u_length, src_uv_stride / 2,
                     src_width / 2, src_height / 2,
                     resize_uv_buf + resize_uv_size / 2,
                     output_second_stride_ / 2,
                     ratio_dst_width_ / 2,
                     ratio_dst_height_ / 2,
                     libyuv::kFilterBox);

  uint8_t *dst_uv = output_uv;
  uint8_t *src_u = &resize_uv_buf[0];
  uint8_t *src_v = &resize_uv_buf[resize_uv_size / 2];
  int stride_u = output_second_stride_ / 2,
      stride_v = output_second_stride_ / 2;
  int halfwidth = ratio_dst_width_ >> 1;
  int halfheight = ratio_dst_height_ >> 1;
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
  HobotXStreamFreeImage(pre_uv_buf);
  HobotXStreamFreeImage(resize_uv_buf);
  // padding right
  if (resize_info_.padding_right_ != 0) {
    uint8_t *output_padding_y = output_y + ratio_dst_width_;
    uint8_t *output_padding_uv = output_uv + ratio_dst_width_;
    for (int i = 0; i < ratio_dst_height_; ++i) {
      memset(output_padding_y, 0, resize_info_.padding_right_);
      output_padding_y += output_first_stride_;
      if ((i & 1) == 0) {
        memset(output_padding_uv, 128, resize_info_.padding_right_);
        output_padding_uv += output_second_stride_;
      }
    }
  }
  // padding bottom
  if (resize_info_.padding_bottom_ != 0) {
    uint8_t *output_padding_y = output_y + \
                  output_first_stride_ * ratio_dst_height_;
    uint8_t *output_padding_uv = output_uv + \
                  output_second_stride_ * ratio_dst_height_ / 2;
    memset(output_padding_y,
           0,
           output_first_stride_ * resize_info_.padding_bottom_);
    memset(output_padding_uv,
           128,
           output_second_stride_ * resize_info_.padding_bottom_ / 2);
  }
  return true;
}

bool ImageResizer::CalcResizeInfo() {
  float width_ratio = static_cast<float>(resize_info_.dst_width_)
                                / resize_info_.src_width_;
  float height_ratio = static_cast<float>(resize_info_.dst_height_)
                              / resize_info_.src_height_;
  if (!resize_info_.fix_aspect_ratio_) {
    resize_info_.padding_right_ = 0;
    resize_info_.padding_bottom_ = 0;
    resize_info_.width_ratio_ = width_ratio;
    resize_info_.height_ratio_ = height_ratio;
    ratio_dst_width_ = resize_info_.dst_width_;
    ratio_dst_height_ = resize_info_.dst_height_;
    return true;
  }
  // fix_aspect_ratio
  float ratio = width_ratio;
  if (ratio > height_ratio) {
    ratio = height_ratio;
  }
  int ratio_dst_width = resize_info_.src_width_ * ratio + 0.5;
  int ratio_dst_height = resize_info_.src_height_ * ratio + 0.5;
  if (IMAGE_TOOLS_RAW_YUV_I420 == input_fmt_
      || IMAGE_TOOLS_RAW_YUV_NV12 == input_fmt_
      || IMAGE_TOOLS_RAW_YUV_NV21 == input_fmt_) {
    ratio_dst_width &= ~1;
    ratio_dst_height &= ~1;
  }
  resize_info_.width_ratio_ = static_cast<float>(ratio_dst_width)
                              / resize_info_.src_width_;
  resize_info_.height_ratio_ = static_cast<float>(ratio_dst_height)
                              / resize_info_.src_height_;
  resize_info_.padding_right_ = resize_info_.dst_width_ - ratio_dst_width;
  resize_info_.padding_bottom_ = resize_info_.dst_height_ - ratio_dst_height;
  ratio_dst_width_ = ratio_dst_width;
  ratio_dst_height_ = ratio_dst_height;
  return true;
}

}  // namespace xstream
