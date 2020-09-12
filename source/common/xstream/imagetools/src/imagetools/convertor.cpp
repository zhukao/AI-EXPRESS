/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides ImageConvertor implement
 * @author    hangjun.yang
 * @email     hangjun.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.13
 */
#include "hobotxstream/imagetools/convertor.h"
#include <libyuv.h>
#include <string.h>
#include <turbojpeg.h>
#include <iostream>
#include <tuple>
#include "hobotxstream/image_tools.h"
#include "opencv2/opencv.hpp"

namespace xstream {

bool ImageConvertor::Convert(const ImageToolsFormatData &input,
                             const HobotXStreamImageToolsPixelFormat dst_fmt,
                             ImageToolsFormatData &output) {
  if (!input.Valid()) {
    return false;
  }
  GetInputImageInfo(input);

  if (input_fmt_ == dst_fmt) {
    return false;
  }

  output_fmt_ = dst_fmt;
  output_width_ = width_ = input_width_;
  output_height_ = height_ = input_height_;

  bool ret = false;
  if (IMAGE_TOOLS_RAW_YUV_I420 == output_fmt_) {
    // dst is i420
    ret = ConvertToI420();
  } else if (IMAGE_TOOLS_RAW_YUV_NV21 == output_fmt_
            || IMAGE_TOOLS_RAW_YUV_NV12 == output_fmt_) {
    // dst is nv12/nv21
    if (IMAGE_TOOLS_RAW_RGB == input_fmt_
        || IMAGE_TOOLS_RAW_BGR == input_fmt_) {
      ret = ConvertRGBOrBGRToNV();
    } else if (IMAGE_TOOLS_RAW_YUV_I420 == input_fmt_) {
      ret = ConvertI420ToNV();
    } else if (IMAGE_TOOLS_RAW_YUV_NV21 == input_fmt_
              || IMAGE_TOOLS_RAW_YUV_NV12 == input_fmt_) {
      ret = ConvertBetweenNV();
    } else {
      ret = false;
    }
  } else if (IMAGE_TOOLS_RAW_RGB == output_fmt_
            || IMAGE_TOOLS_RAW_BGR == output_fmt_) {
    // dst is rgb/bgr
    ret = ConvertToRGBOrBGR();
  } else if (IMAGE_TOOLS_RAW_GRAY == output_fmt_) {
    // dst is gray
    if (IMAGE_TOOLS_RAW_RGB == input_fmt_
        || IMAGE_TOOLS_RAW_BGR == input_fmt_) {
      ret = ConvertRGBorBGRToGray();
    } else if (IMAGE_TOOLS_RAW_YUV_I420 == input_fmt_
               || IMAGE_TOOLS_RAW_YUV_NV21 == input_fmt_
               || IMAGE_TOOLS_RAW_YUV_NV12 == input_fmt_) {
      ret = ConvertYUV420ToGray();
    } else {
      ret = false;
    }
  } else if (IMAGE_TOOLS_RAW_YUV_444 == output_fmt_) {
    if (IMAGE_TOOLS_RAW_YUV_NV12 == input_fmt_) {
      ret = ConvertNV12ToYUV444();
    } else {
      ret = false;
    }
  } else {
    ret = false;
  }
  if (ret) {
    SetOutputImageInfo(output);
  }
  return ret;
}

bool ImageConvertor::ConvertToRGBOrBGR() {
  output_first_stride_ = width_ * 3;
  output_data_size_ = output_first_stride_ * height_;
  int ret = HobotXStreamAllocImage(output_data_size_, &output_data_);
  if (ret != 0) {
    return false;
  }
  if (IMAGE_TOOLS_RAW_YUV_NV21 == input_fmt_
      || IMAGE_TOOLS_RAW_YUV_NV12 == input_fmt_) {
    // from nv21/nv12
    ret = 0;
    const uint8_t *input_y = nullptr;
    const uint8_t *input_uv = nullptr;
    if (FotmatDataArrayType::kContinueType == input_array_type_) {
      input_y = input_data_[0];
      input_uv = input_y + input_first_stride_ * height_;
    } else {
      input_y = input_data_[0];
      input_uv = input_data_[1];
    }

    typedef int (*func_call)(const uint8_t* src_y,
                        int src_stride_y,
                        const uint8_t* src_uv,
                        int src_stride_uv,
                        uint8_t* dst_raw,
                        int dst_stride_raw,
                        int width,
                        int height);
    func_call call = nullptr;
    if (IMAGE_TOOLS_RAW_YUV_NV21 == input_fmt_
        && IMAGE_TOOLS_RAW_RGB == output_fmt_) {
      call = libyuv::NV21ToRAW;
    } else if (IMAGE_TOOLS_RAW_YUV_NV21 == input_fmt_
        && IMAGE_TOOLS_RAW_BGR == output_fmt_) {
      call = libyuv::NV21ToRGB24;
    } else if (IMAGE_TOOLS_RAW_YUV_NV12 == input_fmt_
        && IMAGE_TOOLS_RAW_RGB == output_fmt_) {
      call = libyuv::NV12ToRAW;
    } else if (IMAGE_TOOLS_RAW_YUV_NV12 == input_fmt_
        && IMAGE_TOOLS_RAW_BGR == output_fmt_) {
      call = libyuv::NV12ToRGB24;
    } else {
      ret = -1;
    }
    if (0 == ret) {
      ret = call(input_y, input_first_stride_,
                 input_uv, input_second_stride_,
                 output_data_, output_first_stride_,
                 width_, height_);
    }
  } else if (IMAGE_TOOLS_RAW_RGB == input_fmt_
             || IMAGE_TOOLS_RAW_BGR == input_fmt_) {
    // from rgb/bgr
    ret = libyuv::RAWToRGB24(input_data_[0],
                             input_first_stride_,
                             output_data_,
                             output_first_stride_,
                             width_,
                             height_);
  } else if (IMAGE_TOOLS_RAW_YUV_I420 == input_fmt_) {
    // from i420
    const uint8_t *input_y = nullptr;
    const uint8_t *input_u = nullptr;
    const uint8_t *input_v = nullptr;
    if (FotmatDataArrayType::kContinueType == input_array_type_) {
      input_y = input_data_[0];
      input_u = input_y + input_first_stride_ * height_;
      input_v = input_u + input_second_stride_ * height_ / 2;
    } else {
      input_y = input_data_[0];
      input_u = input_data_[1];
      input_v = input_data_[2];
    }
    if (IMAGE_TOOLS_RAW_RGB == output_fmt_) {
      ret = libyuv::I420ToRAW(input_y, input_first_stride_,
                              input_u, input_second_stride_,
                              input_v, input_second_stride_,
                              output_data_, output_first_stride_,
                              width_, height_);
    } else if (IMAGE_TOOLS_RAW_BGR == output_fmt_) {
      ret = libyuv::I420ToRGB24(input_y, input_first_stride_,
                                input_u, input_second_stride_,
                                input_v, input_second_stride_,
                                output_data_, output_first_stride_,
                                width_, height_);
    } else {
      ret = -1;
    }
  } else {
    ret = -1;
  }
  if (ret != 0) {
    HobotXStreamFreeImage(output_data_);
    output_data_ = nullptr;
    return false;
  } else {
    return true;
  }
}

bool ImageConvertor::ConvertToI420() {
  if (IMAGE_TOOLS_RAW_GRAY == input_fmt_
      || IMAGE_TOOLS_RAW_YUV_I420 == input_fmt_) {
    return false;
  }
  if ((width_ & static_cast<int>(0X01)) != 0
      || (height_ & static_cast<int>(0X01)) != 0) {
    return false;
  }
  output_first_stride_ = width_;
  output_second_stride_ = width_ >> 1;
  output_data_size_ = width_ * height_ * 3 / 2;
  int ret = HobotXStreamAllocImage(output_data_size_, &output_data_);
  if (ret != 0) {
    return false;
  }
  uint8_t *y = output_data_;
  uint8_t *u = y + width_ * height_;
  uint8_t *v = u + width_ * height_ / 4;
  if (IMAGE_TOOLS_RAW_BGR == input_fmt_) {
#ifdef BGR2I420_WITH_LIBYUV
    ret = libyuv::RGB24ToI420(input_data_[0], input_first_stride_,
                              y, output_first_stride_,
                              u, output_second_stride_,
                              v, output_second_stride_,
                              width_, height_);
#else
    cv::Mat mat;
    mat.create(height_, width_, CV_8UC3);
    memcpy(mat.data, input_data_[0], input_data_size_[0]);
    cv::cvtColor(mat, mat, CV_BGR2YUV_I420);
    uint32_t step = mat.step.p[0];
    memcpy(y, mat.data, step * height_);
    memcpy(u, mat.data + step * height_, step * height_ / 4);
    memcpy(v, mat.data + step * height_ * 5 / 4, step * height_ / 4);
    output_first_stride_ = step;
    output_second_stride_ = step / 2;
#endif
  } else if (IMAGE_TOOLS_RAW_RGB == input_fmt_) {
    ret = libyuv::RAWToI420(input_data_[0], input_first_stride_,
                            y, output_first_stride_,
                            u, output_second_stride_,
                            v, output_second_stride_,
                            width_, height_);
  } else if (IMAGE_TOOLS_RAW_YUV_NV12 == input_fmt_) {
    const uint8_t *input_y = nullptr;
    const uint8_t *input_uv = nullptr;
    if (FotmatDataArrayType::kContinueType == input_array_type_) {
      input_y = input_data_[0];
      input_uv = input_y + input_first_stride_ * height_;
    } else {
      input_y = input_data_[0];
      input_uv = input_data_[1];
    }
    ret = libyuv::NV12ToI420(input_y, input_first_stride_,
                             input_uv, input_second_stride_,
                             y, output_first_stride_,
                             u, output_second_stride_,
                             v, output_second_stride_,
                             width_, height_);
  } else if (IMAGE_TOOLS_RAW_YUV_NV21 == input_fmt_) {
    const uint8_t *input_y = nullptr;
    const uint8_t *input_uv = nullptr;
    if (FotmatDataArrayType::kContinueType == input_array_type_) {
      input_y = input_data_[0];
      input_uv = input_y + input_first_stride_ * height_;
    } else {
      input_y = input_data_[0];
      input_uv = input_data_[1];
    }
    ret = libyuv::NV21ToI420(input_y, input_first_stride_,
                             input_uv, input_second_stride_,
                             y, output_first_stride_,
                             u, output_second_stride_,
                             v, output_second_stride_,
                             width_, height_);
  } else {
    ret = -1;
  }
  if (ret != 0) {
    HobotXStreamFreeImage(output_data_);
    output_data_ = nullptr;
    return false;
  } else {
    return true;
  }
}

bool ImageConvertor::ConvertRGBorBGRToGray() {
  int input_pixel_format;
  if (IMAGE_TOOLS_RAW_RGB == input_fmt_) {
    input_pixel_format = TJPF_RGB;
  } else if (IMAGE_TOOLS_RAW_BGR == input_fmt_) {
    input_pixel_format = TJPF_BGR;
  } else {
    return false;
  }
  tjhandle tj_handle = tjInitCompress();
  if (NULL == tj_handle) {
    return false;
  }
  output_data_size_ = width_ * height_;
  int ret = HobotXStreamAllocImage(output_data_size_, &output_data_);
  if (ret != 0) {
    return false;
  }
  output_first_stride_ = width_;
  ret = tjEncodeYUV3(tj_handle,
                         input_data_[0],
                         width_,
                         input_first_stride_,
                         height_,
                         input_pixel_format,
                         output_data_,
                         1,
                         TJSAMP_GRAY,
                         0);
  tjDestroy(tj_handle);
  if (ret != 0) {
    HobotXStreamFreeImage(output_data_);
    output_data_ = nullptr;
    return false;
  } else {
    return true;
  }
}

bool ImageConvertor::ConvertYUV420ToGray() {
  if (input_fmt_ != IMAGE_TOOLS_RAW_YUV_NV21
      && input_fmt_ != IMAGE_TOOLS_RAW_YUV_NV12
      && input_fmt_ != IMAGE_TOOLS_RAW_YUV_I420) {
    return false;
  }
  // 只拷贝Y分量，所以对于NV12或者NV21的uv地址对不对无所谓
  const uint8_t *input_y = input_data_[0];
  const uint8_t *input_u = nullptr;
  const uint8_t *input_v = nullptr;
  output_first_stride_ = width_;
  output_data_size_ = width_ * height_;
  int ret = HobotXStreamAllocImage(output_data_size_, &output_data_);
  if (ret != 0) {
    return false;
  }
  ret = libyuv::I420ToI400(input_y, input_first_stride_,
                               input_u, 0,
                               input_v, 0,
                               output_data_, output_first_stride_,
                               width_,
                               height_);
  if (ret != 0) {
    HobotXStreamFreeImage(output_data_);
    output_data_ = nullptr;
    return false;
  } else {
    return true;
  }
}

bool ImageConvertor::ConvertI420ToNV() {
  if (input_fmt_ != IMAGE_TOOLS_RAW_YUV_I420) {
    return false;
  }
  output_first_stride_ = width_;
  output_second_stride_ = width_;
  output_data_size_ = width_ * height_ * 3 / 2;
  int ret = HobotXStreamAllocImage(output_data_size_, &output_data_);
  if (ret != 0) {
    return false;
  }
  uint8_t *output_y = output_data_;
  uint8_t *output_uv = output_y + output_first_stride_ * height_;
  const uint8_t *input_y = nullptr;
  const uint8_t *input_u = nullptr;
  const uint8_t *input_v = nullptr;
  if (FotmatDataArrayType::kContinueType == input_array_type_) {
    input_y = input_data_[0];
    input_u = input_y + input_first_stride_ * height_;
    input_v = input_u + input_second_stride_ * height_ / 2;
  } else {
    input_y = input_data_[0];
    input_u = input_data_[1];
    input_v = input_data_[2];
  }
  ret = -1;
  if (IMAGE_TOOLS_RAW_YUV_NV21 == output_fmt_) {
    ret = libyuv::I420ToNV21(input_y, input_first_stride_,
                             input_u, input_second_stride_,
                             input_v, input_second_stride_,
                             output_y, output_first_stride_,
                             output_uv, output_second_stride_,
                             width_, height_);
  } else if (IMAGE_TOOLS_RAW_YUV_NV12 == output_fmt_) {
    ret = libyuv::I420ToNV12(input_y, input_first_stride_,
                             input_u, input_second_stride_,
                             input_v, input_second_stride_,
                             output_y, output_first_stride_,
                             output_uv, output_second_stride_,
                             width_, height_);
  } else {
    ret = -1;
  }
  if (ret != 0) {
    HobotXStreamFreeImage(output_data_);
    output_data_ = nullptr;
    return false;
  } else {
    return true;
  }
}

bool ImageConvertor::ConvertRGBOrBGRToNV() {
  if (input_fmt_ != IMAGE_TOOLS_RAW_RGB
      && input_fmt_ != IMAGE_TOOLS_RAW_BGR) {
    return false;
  }
  if (output_fmt_ != IMAGE_TOOLS_RAW_YUV_NV12
      && output_fmt_ != IMAGE_TOOLS_RAW_YUV_NV21) {
    return false;
  }
  if ((width_ & static_cast<int>(0X01)) != 0
      || (height_ & static_cast<int>(0X01)) != 0) {
    return false;
  }
  // first convert to I420, then convert to NV
  int i420_first_stride = width_;
  int i420_second_stride = width_ >> 1;
  int i420_data_size = width_ * height_ * 3 / 2;
  uint8_t *i420_data = nullptr;
  int ret = HobotXStreamAllocImage(i420_data_size, &i420_data);
  if (ret != 0) {
    return false;
  }
  uint8_t *i420_y = i420_data;
  uint8_t *i420_u = i420_y + width_ * height_;
  uint8_t *i420_v = i420_u + width_ * height_ / 4;
  if (IMAGE_TOOLS_RAW_BGR == input_fmt_) {
#ifdef BGR2I420_WITH_LIBYUV
    ret = libyuv::RGB24ToI420(input_data_[0], input_first_stride_,
                              i420_y, i420_first_stride,
                              i420_u, i420_second_stride,
                              i420_v, i420_second_stride,
                              width_, height_);
#else
    cv::Mat mat;
    mat.create(height_, width_, CV_8UC3);
    memcpy(mat.data, input_data_[0], input_data_size_[0]);
    cv::cvtColor(mat, mat, CV_BGR2YUV_I420);
    uint32_t step = mat.step.p[0];
    memcpy(i420_y, mat.data, step * height_);
    memcpy(i420_u, mat.data + step * height_, step * height_ / 4);
    memcpy(i420_v, mat.data + step * height_ * 5 / 4, step * height_ / 4);
    output_first_stride_ = step;
    output_second_stride_ = step / 2;
#endif
  } else {
    ret = libyuv::RAWToI420(input_data_[0], input_first_stride_,
                            i420_y, i420_first_stride,
                            i420_u, i420_second_stride,
                            i420_v, i420_second_stride,
                            width_, height_);
  }
  if (ret != 0) {
    HobotXStreamFreeImage(i420_data);
    return false;
  }

  // i420 to NV12/Nv21
  output_first_stride_ = width_;
  output_second_stride_ = width_;
  output_data_size_ = width_ * height_ * 3 / 2;
  ret = HobotXStreamAllocImage(output_data_size_, &output_data_);
  if (ret != 0) {
    HobotXStreamFreeImage(i420_data);
    return false;
  }
  uint8_t *output_y = output_data_;
  uint8_t *output_uv = output_y + output_first_stride_ * height_;
  if (IMAGE_TOOLS_RAW_YUV_NV21 == output_fmt_) {
    ret = libyuv::I420ToNV21(i420_y, i420_first_stride,
                             i420_u, i420_second_stride,
                             i420_v, i420_second_stride,
                             output_y, output_first_stride_,
                             output_uv, output_second_stride_,
                             width_, height_);
  } else {
    ret = libyuv::I420ToNV12(i420_y, i420_first_stride,
                             i420_u, i420_second_stride,
                             i420_v, i420_second_stride,
                             output_y, output_first_stride_,
                             output_uv, output_second_stride_,
                             width_, height_);
  }
  HobotXStreamFreeImage(i420_data);
  if (ret != 0) {
    HobotXStreamFreeImage(output_data_);
    output_data_ = nullptr;
    return false;
  } else {
    return true;
  }
}

bool ImageConvertor::ConvertBetweenNV() {
  if ((width_ & static_cast<int>(0X01)) != 0
      || (height_ & static_cast<int>(0X01)) != 0) {
    return false;
  }
  output_first_stride_ = width_;
  output_second_stride_ = width_;
  output_data_size_ = width_ * height_ * 3 / 2;
  if (0 != HobotXStreamAllocImage(output_data_size_, &output_data_)) {
    return false;
  }
  uint8_t *output_y = output_data_;
  uint8_t *output_uv = output_y + output_first_stride_ * height_;
  const uint8_t *input_y = nullptr;
  const uint8_t *input_uv = nullptr;
  if (FotmatDataArrayType::kContinueType == input_array_type_) {
    input_y = input_data_[0];
    input_uv = input_y + input_first_stride_ * height_;
  } else {
    input_y = input_data_[0];
    input_uv = input_data_[1];
  }
  if (input_first_stride_ == output_first_stride_) {
    memcpy(output_y, input_y, input_first_stride_ * height_);
  } else {
    for (int h = 0; h < height_; ++h) {
      memcpy(output_y, input_y, width_);
      output_y += output_first_stride_;
      input_y += input_first_stride_;
    }
  }
  for (int h = 0; h < height_; h = h + 2) {
    for (int w = 0; w < width_; w = w + 2) {
      output_uv[w] = input_uv[w + 1];
      output_uv[w + 1] = input_uv[w];
    }
    output_uv += output_second_stride_;
    input_uv += input_second_stride_;
  }
  return true;
}

bool ImageConvertor::ConvertNV12ToYUV444() {
  if (input_fmt_ != IMAGE_TOOLS_RAW_YUV_NV12) {
    return false;
  }
  output_first_stride_ = width_ * 3;
  output_second_stride_ = 0;
  output_data_size_ = width_ * height_ * 3;
  output_data_ = new uint8_t[output_data_size_];

  const uint8_t *input_y = input_data_[0];
  const uint8_t *input_uv = input_y + input_first_stride_ * height_;
  uint8_t *output = output_data_;
  int loop = height_ / 2;
  for (int i = 0; i < loop; i++) {
    for (int j = 0; j < width_; j += 2) {
      *(output++) = *(input_y++);
      *(output++) = *(input_uv + j);
      *(output++) = *(input_uv + j + 1);
      *(output++) = *(input_y++);
      *(output++) = *(input_uv + j);
      *(output++) = *(input_uv + j + 1);
    }
    for (int j = 0; j < width_; j += 2) {
      *(output++) = *(input_y++);
      *(output++) = *(input_uv + j);
      *(output++) = *(input_uv + j + 1);
      *(output++) = *(input_y++);
      *(output++) = *(input_uv + j);
      *(output++) = *(input_uv + j + 1);
    }
    input_uv += width_;
  }
  return true;
}


}  // namespace xstream
