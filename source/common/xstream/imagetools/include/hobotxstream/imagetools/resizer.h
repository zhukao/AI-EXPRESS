/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream image Resizer
 * @author    hangjun.yang
 * @email     hangjun.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.12
 */

#ifndef INCLUDE_HOBOTXSTREAM_IMAGETOOLS_RESIZER_H_
#define INCLUDE_HOBOTXSTREAM_IMAGETOOLS_RESIZER_H_

#include "hobotxstream/imagetools/common.h"
#include "hobotxstream/imagetools/base.h"

namespace xstream {

class ImageResizer : public ImageBase {
 public:
  bool Resize(const ImageToolsFormatData &input,
              const int dst_width,
              const int dst_height,
              const bool fix_aspect_ratio,
              ImageToolsFormatData &output);

  inline HobotXStreamImageToolsResizeInfo GetResizeInfo() {
    return resize_info_;
  }

 private:
  bool ResizeI420();

  bool ResizeOnePlane();

  bool ResizeNV();

  // 计算获取缩放信息
  bool CalcResizeInfo();

 private:
  HobotXStreamImageToolsResizeInfo resize_info_;
  int ratio_dst_width_;  // src_width * resize_info_.width_ratio_
  int ratio_dst_height_;
};
}  // namespace xstream

#endif  // INCLUDE_HOBOTXSTREAM_IMAGETOOLS_RESIZER_H_
