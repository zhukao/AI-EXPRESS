/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream image Cropper implement
 * @author    hangjun.yang
 * @email     hangjun.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.14
 */
#include <string.h>
#include <cmath>
#include "hobotxstream/imagetools/cropper.h"
#include "hobotxstream/image_tools.h"
#include "hobotxstream/imagetools/padding.h"
namespace xstream {

bool ImageCropper::Crop(const ImageToolsFormatData &input,
                        const int top_left_x,
                        const int top_left_y,
                        const int bottom_right_x,
                        const int bottom_right_y,
                        ImageToolsFormatData &output) {
  GetInputImageInfo(input);
  if (top_left_x < 0 || top_left_y < 0) {
    return false;
  }
  if (bottom_right_x < top_left_x || bottom_right_y < top_left_y) {
    return false;
  }
  if (bottom_right_x >= input_width_ || bottom_right_y >= input_height_) {
    return false;
  }
  top_left_x_ = top_left_x;
  top_left_y_ = top_left_y;
  bottom_right_x_ = bottom_right_x;
  bottom_right_y_ = bottom_right_y;
  output_width_ = bottom_right_x - top_left_x + 1;
  output_height_ = bottom_right_y - top_left_y + 1;
  if (!AlignEven()) {
    return false;
  }
  output_fmt_ = input_fmt_;
  bool ret = false;
  if (IMAGE_TOOLS_RAW_RGB == input_fmt_
      || IMAGE_TOOLS_RAW_BGR == input_fmt_
      || IMAGE_TOOLS_RAW_GRAY == input_fmt_) {
    ret = CropRgbOrBgrOrGray();
  } else if (IMAGE_TOOLS_RAW_YUV_I420 == input_fmt_) {
    ret = CropI420();
  } else if (IMAGE_TOOLS_RAW_YUV_NV21 == input_fmt_
            || IMAGE_TOOLS_RAW_YUV_NV12 == input_fmt_) {
    ret = CropNV();
  } else {
    return false;
  }
  if (!ret) {
    return false;
  } else {
    SetOutputImageInfo(output);
    return true;
  }
}

bool ImageCropper::CropWithPadBlack(const ImageToolsFormatData &input,
                                    const int top_left_x,
                                    const int top_left_y,
                                    const int bottom_right_x,
                                    const int bottom_right_y,
                                    ImageToolsFormatData &output) {
  if (bottom_right_x < top_left_x || bottom_right_y < top_left_y) {
    return false;
  }
  int crop_top_left_x = top_left_x;
  int crop_top_left_y = top_left_y;
  int crop_bottom_right_x = bottom_right_x;
  int crop_bottom_right_y = bottom_right_y;

  int padding_top = 0;
  int padding_left = 0;
  int padding_right = 0;
  int padding_bottom = 0;
  if (crop_top_left_x < 0) {
    padding_left = std::abs(crop_top_left_x);
    crop_top_left_x = 0;
  }
  if (crop_top_left_y < 0) {
    padding_top = std::abs(crop_top_left_y);
    crop_top_left_y = 0;
  }
  if (crop_bottom_right_x > (input.width_ - 1)) {
    padding_right = crop_bottom_right_x - input.width_ + 1;
    crop_bottom_right_x = input.width_ - 1;
  }
  if (crop_bottom_right_y > (input.height_ - 1)) {
    padding_bottom = crop_bottom_right_y - input.height_ + 1;
    crop_bottom_right_y = input.height_ - 1;
  }
  bool need_padding = true;
  bool need_crop = false;
  if (crop_bottom_right_x > crop_top_left_x
      && crop_bottom_right_y > crop_top_left_y) {
    need_crop = true;
  }
  if (0 == padding_top
      && 0 == padding_left
      && 0 == padding_right
      && 0 == padding_bottom) {
    need_padding = false;
  }
  if (!need_padding) {
    return Crop(input,
                top_left_x,
                top_left_y,
                bottom_right_x,
                bottom_right_y,
                output);
  } else {
    ImagePadding pad;
    uint8_t padding_value[3];
    if (IMAGE_TOOLS_RAW_RGB == input.format_ ||
        IMAGE_TOOLS_RAW_BGR == input.format_ ||
        IMAGE_TOOLS_RAW_GRAY == input.format_) {
      padding_value[0] = padding_value[1] = padding_value[2] = 0;
    } else {
      // yuv 420
      padding_value[0] = 0;
      padding_value[1] = padding_value[2] = 128;
    }
    if (need_crop) {
      ImageToolsFormatData tmp_data;
      if (!Crop(input,
                crop_top_left_x,
                crop_top_left_y,
                crop_bottom_right_x,
                crop_bottom_right_y,
                tmp_data)) {
        return false;
      }
      if (IMAGE_TOOLS_RAW_YUV_I420 == input_fmt_
      || IMAGE_TOOLS_RAW_YUV_NV21 == input_fmt_
      || IMAGE_TOOLS_RAW_YUV_NV12 == input_fmt_) {
        padding_left = AlignEven(padding_left);
        padding_right = AlignEven(padding_right);
        padding_top = AlignEven(padding_top);
        padding_bottom = AlignEven(padding_bottom);
      }
      bool ret = pad.Pad(tmp_data,
                         padding_left,
                         padding_right,
                         padding_top,
                         padding_bottom,
                         padding_value,
                         output);
      HobotXStreamFreeImage(tmp_data.data_[0]);
      return ret;
    } else {
      return pad.Pad(input,
                     padding_left,
                     padding_right,
                     padding_top,
                     padding_bottom,
                     padding_value,
                     output);
    }
  }
}

bool ImageCropper::CropRgbOrBgrOrGray() {
  int element_size = 1;
  if (IMAGE_TOOLS_RAW_GRAY == input_fmt_) {
    element_size = 1;
  } else {
    element_size = 3;
  }
  output_first_stride_ = output_width_ * element_size;
  output_data_size_ = output_first_stride_ * output_height_;
  if (0 != HobotXStreamAllocImage(output_data_size_, &output_data_)) {
    return false;
  }
  bool ret = CropOnePlane(input_data_[0],
                          input_first_stride_,
                          element_size,
                          output_data_,
                          output_first_stride_);
  if (!ret) {
    HobotXStreamFreeImage(output_data_);
    output_data_ = nullptr;
  }
  return ret;
}

bool ImageCropper::CropI420() {
  if ((output_width_ & static_cast<int>(0X01)) != 0
      || (output_height_ & static_cast<int>(0X01)) != 0) {
    return false;
  }
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

  // crop y
  bool ret = CropOnePlane(input_y,
                          input_first_stride_,
                          1,
                          output_y,
                          output_first_stride_);
  if (!ret) {
    HobotXStreamFreeImage(output_data_);
    output_data_ = nullptr;
    return false;
  }
  input_u = input_u + (top_left_y_ / 2) * input_second_stride_\
            + top_left_x_ / 2;
  input_v = input_v + (top_left_y_ / 2) * input_second_stride_\
            + top_left_x_ / 2;
  // crop u,v
  for (int y = 0; y < output_height_; y = y + 2) {
    memcpy(output_u, input_u, output_second_stride_);
    memcpy(output_v, input_v, output_second_stride_);
    output_u += output_second_stride_;
    output_v += output_second_stride_;
    input_u += input_second_stride_;
    input_v += input_second_stride_;
  }
  return true;
}

bool ImageCropper::CropNV() {
  if ((output_width_ & static_cast<int>(0X01)) != 0
      || (output_height_ & static_cast<int>(0X01)) != 0) {
    return false;
  }
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

  uint8_t *output_y = output_data_;
  uint8_t *output_uv = output_y + output_first_stride_ * output_height_;

  // crop y
  bool ret = CropOnePlane(input_y,
                          input_first_stride_,
                          1,
                          output_y,
                          output_first_stride_);
  if (!ret) {
    HobotXStreamFreeImage(output_data_);
    output_data_ = nullptr;
    return false;
  }
  input_uv = input_uv + (top_left_y_ / 2) * input_second_stride_
             + (top_left_x_ & (~1));
  // crop uv
  for (int y = 0; y < output_height_; y = y + 2) {
    memcpy(output_uv, input_uv, output_second_stride_);
    output_uv += output_second_stride_;
    input_uv += input_second_stride_;
  }
  return true;
}

bool ImageCropper::CropOnePlane(const uint8_t *input_data,
                                const int input_stride,
                                const int element_size,
                                uint8_t *output_data,
                                const int output_stride) {
  const uint8_t *input_pos = input_data
         + top_left_y_ * input_stride + top_left_x_ * element_size;
  uint8_t *output_pos = output_data;
  for (int y = 0; y < output_height_; ++y) {
    memcpy(output_pos, input_pos, output_stride);
    output_pos += output_stride;
    input_pos += input_stride;
  }
  return true;
}

bool ImageCropper::AlignEven() {
  if (IMAGE_TOOLS_RAW_YUV_I420 == input_fmt_
      || IMAGE_TOOLS_RAW_YUV_NV21 == input_fmt_
      || IMAGE_TOOLS_RAW_YUV_NV12 == input_fmt_) {
    if (output_width_ & (static_cast<int>(0X01) != 0)) {
      if (bottom_right_x_ < (input_width_ - 1)) {
        ++bottom_right_x_;
      } else if (top_left_x_ > 0) {
        --top_left_x_;
      } else {
        return false;
      }
      ++output_width_;
    }
    if (output_height_ & (static_cast<int>(0X01) != 0)) {
      if (bottom_right_y_ < (input_height_ - 1)) {
        ++bottom_right_y_;
      } else if (top_left_y_ > 0) {
        --top_left_y_;
      } else {
        return false;
      }
      ++output_height_;
    }
  }
  return true;
}

int ImageCropper::AlignEven(int value) {
  if (value & (static_cast<int>(0X01) != 0)) {
    ++value;
  }
  return value;
}

}  // namespace xstream
