/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream image Padding implement
 * @author    hangjun.yang
 * @email     hangjun.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.15
 */

#include "hobotxstream/imagetools/padding.h"
#include <string.h>
#include <libyuv.h>
#include <iostream>

namespace xstream {
bool ImagePadding::Pad(const ImageToolsFormatData &input,
                       const int padding_left_width,
                       const int padding_right_width,
                       const int padding_top_height,
                       const int padding_bottom_height,
                       const uint8_t* padding_value,
                       ImageToolsFormatData &output) {
  if (!input.Valid()) {
    return false;
  }
  GetInputImageInfo(input);
  if (padding_left_width < 0 || padding_right_width < 0
      || padding_top_height < 0 || padding_bottom_height < 0) {
    return false;
  }

  padding_left_width_ = padding_left_width;
  padding_right_width_ = padding_right_width;
  padding_top_height_ = padding_top_height;
  padding_bottom_height_ = padding_bottom_height;
  output_width_ = input_width_ + padding_left_width_ + padding_right_width_;
  output_height_ = input_height_ + padding_top_height_ + padding_bottom_height_;
  output_fmt_ = input_fmt_;
  padding_value_ = padding_value;
  bool ret = false;
  if (IMAGE_TOOLS_RAW_RGB == input_fmt_
      || IMAGE_TOOLS_RAW_BGR == input_fmt_
      || IMAGE_TOOLS_RAW_GRAY == input_fmt_) {
    ret = PadOnePlane();
  } else if (IMAGE_TOOLS_RAW_YUV_I420 == input_fmt_) {
    ret = PadI420();
  } else if (IMAGE_TOOLS_RAW_YUV_NV12 == input_fmt_
            || IMAGE_TOOLS_RAW_YUV_NV21 == input_fmt_) {
    ret = PadNV();
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

void ImagePadding::PadRow(uint8_t *data,
                         const int element_size,
                         const int width,
                         const uint8_t *padding_value) {
  if (1 == element_size) {
      memset(data, padding_value[0], width);
  } else {
    for (int w = 0; w < width; w++) {
      for (int i = 0; i < element_size; ++i) {
        data[i] = padding_value[i];
      }
      data += element_size;
    }
  }
}

bool ImagePadding::PadOnePlane() {
  int element_size = 1;
  uint8_t padding_value[3];
  if (IMAGE_TOOLS_RAW_GRAY == input_fmt_) {
    element_size = 1;
    output_first_stride_ = output_width_;
    padding_value[0] = padding_value_[0];
  } else {
    // must be IMAGE_TOOLS_RAW_RGB or IMAGE_TOOLS_RAW_BGR
    element_size = 3;
    output_first_stride_ = output_width_ * 3;
    if (IMAGE_TOOLS_RAW_RGB == input_fmt_) {
      padding_value[0] = padding_value_[0];
      padding_value[1] = padding_value_[1];
      padding_value[2] = padding_value_[2];
    } else if (IMAGE_TOOLS_RAW_BGR == input_fmt_) {
      padding_value[0] = padding_value_[2];
      padding_value[1] = padding_value_[1];
      padding_value[2] = padding_value_[0];
    }
  }
  output_data_size_ = output_first_stride_ * output_height_;
  if (0 != HobotXStreamAllocImage(output_data_size_, &output_data_)) {
    return false;
  }
  uint8_t *output_row_data = output_data_;
  const uint8_t *input_row_data = input_data_[0];
  // padding top
  for (int h = 0; h < padding_top_height_; ++h) {
    PadRow(output_row_data, element_size, output_width_, padding_value);
    output_row_data += output_first_stride_;
  }
  // padding left & right
  for (int h = 0; h < input_height_; ++h) {
    // padding left
    if (padding_left_width_ > 0) {
      PadRow(output_row_data,
             element_size,
             padding_left_width_,
             padding_value);
      output_row_data += padding_left_width_ * element_size;
    }
    // copy origin
    memcpy(output_row_data, input_row_data, input_width_ * element_size);
    output_row_data += input_width_ * element_size;
    input_row_data += input_first_stride_;

    // padding right
    if (padding_right_width_ > 0) {
      PadRow(output_row_data,
             element_size,
             padding_right_width_,
             padding_value);
      output_row_data += padding_right_width_ * element_size;
    }
  }
  // padding bottom
  for (int h = 0; h < padding_bottom_height_; ++h) {
    PadRow(output_row_data, element_size, output_width_, padding_value);
    output_row_data += output_first_stride_;
  }
  return true;
}

bool ImagePadding::PadI420() {
  if ((output_width_ & static_cast<int>(0X01)) != 0
      || (output_height_ & static_cast<int>(0X01)) != 0) {
    return false;
  }
  if ((padding_left_width_ & static_cast<int>(0X01)) != 0
      || (padding_top_height_ & static_cast<int>(0X01)) != 0) {
    return false;
  }
  output_first_stride_ = output_width_;
  output_second_stride_ = output_width_ >> 1;
  output_data_size_ = output_width_ * output_height_ * 3 / 2;
  if (0 != HobotXStreamAllocImage(output_data_size_, &output_data_)) {
    return false;
  }
  uint8_t *output_y = output_data_;
  uint8_t *output_u = output_y + output_first_stride_ * output_height_;
  uint8_t *output_v = output_u + output_second_stride_ * output_height_ / 2;

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

  uint8_t value_y[3] = {padding_value_[0], 0, 0};
  uint8_t value_u[3] = {padding_value_[1], 0, 0};
  uint8_t value_v[3] = {padding_value_[2], 0, 0};

  // padding top
  for (int h = 0; h < padding_top_height_; ++h) {
    // padding y
    PadRow(output_y, 1, output_width_, value_y);
    output_y += output_first_stride_;

    // padding u && v
    if ((h & 1) == 0) {
      PadRow(output_u, 1, output_width_ / 2, value_u);
      PadRow(output_v, 1, output_width_ / 2, value_v);
      output_u += output_second_stride_;
      output_v += output_second_stride_;
    }
  }

  // padding left & right
  for (int h = 0; h < input_height_; ++h) {
    // padding left
    if (padding_left_width_ > 0) {
      // padding y
      PadRow(output_y,
             1,
             padding_left_width_,
             value_y);
      output_y += padding_left_width_;

      // padding u && v
      if ((h & 1) == 0) {
        PadRow(output_u, 1, padding_left_width_ / 2, value_u);
        PadRow(output_v, 1, padding_left_width_ / 2, value_v);
        output_u += padding_left_width_ / 2;
        output_v += padding_left_width_ / 2;
      }
    }

    // copy origin
    // copy y
    memcpy(output_y, input_y, input_width_);
    output_y += input_width_;
    input_y += input_first_stride_;
    // copy u & v
    if ((h & 1) == 0) {
      memcpy(output_u, input_u, input_width_ / 2);
      memcpy(output_v, input_v, input_width_ / 2);
      output_u += input_width_ / 2;
      output_v += input_width_ / 2;
      input_u += input_second_stride_;
      input_v += input_second_stride_;
    }

    // padding right
    if (padding_right_width_ > 0) {
      // padding y
      PadRow(output_y,
             1,
             padding_right_width_,
             value_y);
      output_y += padding_right_width_;

      // padding u && v
      if ((h & 1) == 0) {
        PadRow(output_u, 1, padding_right_width_ / 2, value_u);
        PadRow(output_v, 1, padding_right_width_ / 2, value_v);
        output_u += padding_right_width_ / 2;
        output_v += padding_right_width_ / 2;
      }
    }
  }

  // padding bottom
  for (int h = 0; h < padding_bottom_height_; ++h) {
    // padding y
    PadRow(output_y, 1, output_width_, value_y);
    output_y += output_first_stride_;

    // padding u && v
    if ((h & 1) == 0) {
      PadRow(output_u, 1, output_width_ / 2, value_u);
      PadRow(output_v, 1, output_width_ / 2, value_v);
      output_u += output_second_stride_;
      output_v += output_second_stride_;
    }
  }
  return true;
}

bool ImagePadding::PadNV() {
  if ((output_width_ & static_cast<int>(0X01)) != 0
      || (output_height_ & static_cast<int>(0X01)) != 0) {
    return false;
  }
  if ((padding_left_width_ & static_cast<int>(0X01)) != 0
      || (padding_top_height_ & static_cast<int>(0X01)) != 0) {
    return false;
  }
  output_first_stride_ = output_width_;
  output_second_stride_ = output_width_;
  output_data_size_ = output_width_ * output_height_ * 3 / 2;
  if (0 != HobotXStreamAllocImage(output_data_size_, &output_data_)) {
    return false;
  }
  uint8_t *output_y = output_data_;
  uint8_t *output_uv = output_y + output_first_stride_ * output_height_;
  const uint8_t *input_y = nullptr;
  const uint8_t *input_uv = nullptr;
  if (FotmatDataArrayType::kContinueType == input_array_type_) {
    input_y = input_data_[0];
    input_uv = input_y + input_first_stride_ * input_height_;
  } else {
    input_y = input_data_[0];
    input_uv = input_data_[1];
  }

  uint8_t value_y[3] = {padding_value_[0], 0, 0};
  uint8_t value_uv[3] = {0, 0, 0};
  if (IMAGE_TOOLS_RAW_YUV_NV12 == input_fmt_) {
    value_uv[0] = padding_value_[1];
    value_uv[1] = padding_value_[2];
  } else if (IMAGE_TOOLS_RAW_YUV_NV21 == input_fmt_) {
    value_uv[0] = padding_value_[2];
    value_uv[1] = padding_value_[1];
  }
  // padding top
  for (int h = 0; h < padding_top_height_; ++h) {
    // padding y
    PadRow(output_y, 1, output_width_, value_y);
    output_y += output_first_stride_;

    // padding u && v
    if ((h & 1) == 0) {
      PadRow(output_uv, 2, output_width_ / 2, value_uv);
      output_uv += output_second_stride_;
    }
  }

  // padding left & right
  for (int h = 0; h < input_height_; ++h) {
    // padding left
    if (padding_left_width_ > 0) {
      // padding y
      PadRow(output_y,
             1,
             padding_left_width_,
             value_y);
      output_y += padding_left_width_;

      // padding u && v
      if ((h & 1) == 0) {
        PadRow(output_uv, 2, padding_left_width_ / 2, value_uv);
        output_uv += padding_left_width_;
      }
    }

    // copy origin
    // copy y
    memcpy(output_y, input_y, input_width_);
    output_y += input_width_;
    input_y += input_first_stride_;
    // copy uv
    if ((h & 1) == 0) {
      memcpy(output_uv, input_uv, input_width_);
      output_uv += input_width_;
      input_uv += input_second_stride_;
    }

    // padding right
    if (padding_right_width_ > 0) {
      // padding y
      PadRow(output_y,
             1,
             padding_right_width_,
             value_y);
      output_y += padding_right_width_;

      // padding u && v
      if ((h & 1) == 0) {
        PadRow(output_uv, 2, padding_right_width_ / 2, value_uv);
        output_uv += padding_right_width_;
      }
    }
  }

  // padding bottom
  for (int h = 0; h < padding_bottom_height_; ++h) {
    // padding y
    PadRow(output_y, 1, output_width_, value_y);
    output_y += output_first_stride_;

    // padding u && v
    if ((h & 1) == 0) {
      PadRow(output_uv, 2, output_width_ / 2, value_uv);
      output_uv += output_second_stride_;
    }
  }
  return true;
}
}  // namespace xstream
