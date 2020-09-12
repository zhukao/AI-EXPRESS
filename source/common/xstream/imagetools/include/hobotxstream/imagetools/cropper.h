/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream image Cropper definition
 * @author    hangjun.yang
 * @email     hangjun.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.14
 */

#ifndef INCLUDE_HOBOTXSTREAM_IMAGETOOLS_CROPPER_H_
#define INCLUDE_HOBOTXSTREAM_IMAGETOOLS_CROPPER_H_

#include "hobotxstream/imagetools/common.h"
#include "hobotxstream/imagetools/base.h"

namespace xstream {
class ImageCropper : public ImageBase {
 public:
  bool Crop(const ImageToolsFormatData &input,
            const int top_left_x,
            const int top_left_y,
            const int bottom_right_x,
            const int bottom_right_y,
            ImageToolsFormatData &output);

  bool CropWithPadBlack(const ImageToolsFormatData &input,
            const int top_left_x,
            const int top_left_y,
            const int bottom_right_x,
            const int bottom_right_y,
            ImageToolsFormatData &output);

 private:
  // crop BGR/RGB/GRAY
  bool CropRgbOrBgrOrGray();

  bool CropI420();

  bool CropNV();

  bool CropOnePlane(const uint8_t *input_data,
                    const int stride,
                    const int element_size,
                    uint8_t *output_data,
                    const int output_stride);

  bool AlignEven();

  int AlignEven(int value);

 private:
  int top_left_x_ = 0;
  int top_left_y_ = 0;
  int bottom_right_x_ = 0;
  int bottom_right_y_ = 0;
};


}  // namespace xstream


#endif  // INCLUDE_HOBOTXSTREAM_IMAGETOOLS_CROPPER_H_
