/**
* Copyright (c) 2019 Horizon Robotics. All rights reserved.
* @file util.h
* @brief image conversion to xroc struct declaration
* @author ruoting.ding
* @email ruoting.ding@horizon.ai
* @date 2019/4/29
*/

#ifndef INCLUDE_HORIZON_VISION_UTIL_H_
#define INCLUDE_HORIZON_VISION_UTIL_H_
#include <cstdint>
#include <memory>

#include "horizon/vision_type/vision_type.h"
#include "horizon/vision_type/vision_type.hpp"
#include "hobotxsdk/xstream_sdk.h"

using hobot::vision::ImageFrame;
using hobot::vision::BBox;

namespace horizon {
namespace vision {
namespace util {

using ImageFramePtr = std::shared_ptr<hobot::vision::ImageFrame>;
using XStreamImageFramePtr = xstream::XStreamData<ImageFramePtr>;
using XStreamBaseDataVectorPtr = std::shared_ptr<xstream::BaseDataVector>;

HorizonVisionImage *ImageConversion(uint8_t *data,
                                    int data_size,
                                    int width,
                                    int height,
                                    int stride,
                                    int stride_uv,
                                    HorizonVisionPixelFormat format);

HorizonVisionImage *ImageConversion(const ImageFramePtr &cpp_img);
XStreamImageFramePtr *ImageConversion(const HorizonVisionImage &c_img);
XStreamImageFramePtr *ImageFrameConversion(const HorizonVisionImageFrame *);

/**
 * @brief 将图像buffer转换成opencv的mat格式
 *
 * @param buffer
 * @param width
 * @param height
 * @param format
 * @param cv_mat
 *
 * @return 0 success; other failed
 */
int Buffer2CVImage(unsigned char *buffer, int width, int height,
  const HorizonVisionPixelFormat &format, cv::Mat *cv_mat);

/**
 * @brief 扣图并resize到指定的大小
 *
 * @param frame
 * @param crop_rect
 * @param output_width
 * @param output_height
 * @param need_resize
 *
 * @return nullptr failed; other 扣图
 */
std::shared_ptr<ImageFrame> CropResize(const std::shared_ptr<ImageFrame> &frame,
                                    const hobot::vision::BBox_<int> &crop_rect,
                                    const uint32_t &output_width,
                                    const uint32_t &output_height,
                                    const bool &need_resize);

}  // namespace util
}  // namespace vision
}  // namespace horizon

#endif  //  INCLUDE_HORIZON_VISION_UTIL_H_
