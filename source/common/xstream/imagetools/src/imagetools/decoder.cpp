/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream image tools interface
 * @author    hangjun.yang
 * @email     hangjun.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.10
 */
#include "hobotxstream/imagetools/decoder.h"
#include <string.h>
#include <turbojpeg.h>
#include <iostream>
#include <tuple>
#include "hobotxstream/imagetools/convertor.h"
#include "hobotxstream/image_tools.h"

namespace xstream {

bool ImageDecoder::DecodeByTj(TJPF eformat) {
  if (nullptr == input_data_ || input_data_size_ <= 0) {
    return false;
  }
  tjhandle tj_handle = tjInitDecompress();
  if (NULL == tj_handle) {
    return false;
  }
  int jpeg_subsamp = 0;
  int jpeg_color_space = 0;
  if (0 != tjDecompressHeader3(tj_handle,
                               input_data_,
                               input_data_size_,
                               &width_,
                               &height_,
                               &jpeg_subsamp,
                               &jpeg_color_space)) {
    tjDestroy(tj_handle);
    return false;
  }
  if (width_ <= 0 || height_ <= 0) {
    tjDestroy(tj_handle);
    return false;
  }
  // make sure output image width and height is 2N
  width_ = width_ + (width_ & 1);
  height_ = height_ + (height_ & 1);

  first_stride_ = tjPixelSize[eformat] * width_;
  output_data_size_ = first_stride_ * height_;
  if (0 != HobotXStreamAllocImage(output_data_size_, &output_data_)) {
    return false;
  }
  if (0 != tjDecompress2(tj_handle,
                         input_data_,
                         input_data_size_,
                         output_data_,
                         width_,
                         first_stride_,
                         height_,
                         eformat,
                         0)) {
    tjDestroy(tj_handle);
    HobotXStreamFreeImage(output_data_);
    output_data_ = nullptr;
    return false;
  }
  tjDestroy(tj_handle);
  return true;
}

bool ImageDecoder::Decode(const uint8_t *input,
                          const int input_size,
                          const HobotXStreamImageToolsPixelFormat dst_fmt,
                          ImageToolsFormatData &output) {
  if (nullptr == input || input_size <= 0) {
    return false;
  }
  input_data_ = input;
  input_data_size_ = input_size;
  dst_fmt_ = dst_fmt;
  bool ret = false;
  if (IMAGE_TOOLS_RAW_RGB == dst_fmt_) {
    ret = DecodeByTj(TJPF_RGB);
  } else if (IMAGE_TOOLS_RAW_BGR == dst_fmt_) {
    ret = DecodeByTj(TJPF_BGR);
  } else if (IMAGE_TOOLS_RAW_GRAY == dst_fmt_) {
    ret = DecodeByTj(TJPF_GRAY);
  } else if (IMAGE_TOOLS_RAW_YUV_I420 == dst_fmt_
            || IMAGE_TOOLS_RAW_YUV_NV12 == dst_fmt_
            || IMAGE_TOOLS_RAW_YUV_NV21 == dst_fmt_) {
    ret = DecodeByTj(TJPF_BGR);
  } else {
    return false;
  }
  if (!ret) {
    return false;
  }

  if (IMAGE_TOOLS_RAW_RGB == dst_fmt_
      || IMAGE_TOOLS_RAW_BGR == dst_fmt_
      || IMAGE_TOOLS_RAW_GRAY == dst_fmt_) {
    output.data_[0] = output_data_;
    output.data_size_[0] = output_data_size_;
    output.width_ = width_;
    output.height_ = height_;
    output.first_stride_ = first_stride_;
    output.format_ = dst_fmt_;
  } else if (IMAGE_TOOLS_RAW_YUV_I420 == dst_fmt_
             || IMAGE_TOOLS_RAW_YUV_NV12 == dst_fmt_
             || IMAGE_TOOLS_RAW_YUV_NV21 == dst_fmt_) {
    ImageToolsFormatData output_bgr;
    output_bgr.data_[0] = output_data_;
    output_bgr.data_size_[0] = output_data_size_;
    output_bgr.width_ = width_;
    output_bgr.height_ = height_;
    output_bgr.first_stride_ = first_stride_;
    output_bgr.format_ = IMAGE_TOOLS_RAW_BGR;
    ImageConvertor convert;
    ret = convert.Convert(output_bgr, dst_fmt_, output);
    HobotXStreamFreeImage(output_bgr.data_[0]);
  }
  return ret;
}

}  // namespace xstream
