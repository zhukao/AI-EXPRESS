/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file util.h
 * @brief image conversion to xstream struct declaration
 * @author ruoting.ding
 * @email ruoting.ding@horizon.ai
 * @date 2019/4/29
 */

#ifndef INCLUDE_HORIZON_VISION_UTIL_H_
#define INCLUDE_HORIZON_VISION_UTIL_H_
#include <cstdint>
#include <memory>

#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision_type/vision_type.h"
#include "horizon/vision_type/vision_type.hpp"

namespace horizon {
namespace vision {
namespace util {

using ImageFramePtr = std::shared_ptr<hobot::vision::ImageFrame>;
using XStreamImageFramePtr = xstream::XStreamData<ImageFramePtr>;
using XStreamBaseDataVectorPtr = std::shared_ptr<xstream::BaseDataVector>;

HorizonVisionImage *ImageConversion(uint8_t *data, int data_size, int width,
                                    int height, int stride, int stride_uv,
                                    HorizonVisionPixelFormat format);

HorizonVisionImage *ImageConversion(const ImageFramePtr &cpp_img);
XStreamImageFramePtr *ImageConversion(const HorizonVisionImage &c_img);
XStreamImageFramePtr *ImageFrameConversion(const HorizonVisionImageFrame *);

}  // namespace util
}  // namespace vision
}  // namespace horizon

#endif  //  INCLUDE_HORIZON_VISION_UTIL_H_
