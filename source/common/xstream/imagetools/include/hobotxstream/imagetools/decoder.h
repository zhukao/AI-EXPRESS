/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream image Decoder
 * @author    hangjun.yang
 * @email     hangjun.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.10
 */
#ifndef INCLUDE_HOBOTXSTREAM_IMAGETOOLS_DECODER_H_
#define INCLUDE_HOBOTXSTREAM_IMAGETOOLS_DECODER_H_

#include <turbojpeg.h>
#include "hobotxstream/imagetools/common.h"
#include "hobotxstream/image_tools.h"

namespace xstream {

class ImageDecoder {
 public:
  bool Decode(const uint8_t *input,
              const int input_size,
              const HobotXStreamImageToolsPixelFormat dst_fmt,
              ImageToolsFormatData &output);
 private:
  // decode to rgb/bgr/gray
  bool DecodeByTj(TJPF eformat);

 private:
  const uint8_t *input_data_ = nullptr;
  int input_data_size_ = 0;
  HobotXStreamImageToolsPixelFormat dst_fmt_;
  int width_ = 0;
  int height_ = 0;
  uint8_t *output_data_ = nullptr;
  int output_data_size_ = 0;
  int first_stride_ = 0;
  int second_stride_ = 0;
};
}  // namespace xstream

#endif  // INCLUDE_HOBOTXSTREAM_IMAGETOOLS_DECODER_H_
