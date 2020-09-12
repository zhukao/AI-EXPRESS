/**
* Copyright (c) 2019 Horizon Robotics. All rights reserved.
* @file util.h
* @brief convert common image to xstream struct.
* @author ruoting.ding
* @email ruoting.ding@horizon.ai
* @date 2019/4/29
*/

#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include "horizon/vision_type/vision_type.h"
#include "horizon/vision_type/vision_type.hpp"
#include "horizon/vision_type/vision_type_util.h"
#include "hobotlog/hobotlog.hpp"
#include "opencv2/opencv.hpp"
#include "horizon/vision/util.h"
#include "hobotxsdk/xstream_data.h"

namespace horizon {
namespace vision {
namespace util {

#ifdef X2
void check_pyramid(img_info_t *img_info) {
  for (auto i = 0; i < 6; ++i) {
    auto index = i * 4;
    HOBOT_CHECK(img_info->down_scale[index].height);
    HOBOT_CHECK(img_info->down_scale[index].step);
  }
}
#endif

#ifdef X3
void check_pyramid(hobot::vision::PymImageFrame *img_info) {
  for (auto i = 0; i < 6; ++i) {
    auto index = i * 4;
    HOBOT_CHECK(img_info->down_scale[index].height);
    HOBOT_CHECK(img_info->down_scale[index].stride);
  }
}
#endif

XStreamImageFramePtr *ImageConversion(const HorizonVisionImage &c_img) {
  auto xstream_img = new XStreamImageFramePtr();
  xstream_img->type_ = "ImageFrame";
  auto image_type = c_img.pixel_format;
  switch (image_type) {
    case kHorizonVisionPixelFormatNone: {
      LOGI << "kHorizonVisionPixelFormatNone, data size is " << c_img.data_size;
      auto cv_img = std::make_shared<hobot::vision::CVImageFrame>();
      std::vector<unsigned char>
          buf(c_img.data, c_img.data + c_img.data_size);
      cv_img->img =
          cv::imdecode(buf, cv::IMREAD_COLOR);
      HOBOT_CHECK(!cv_img->img.empty())
      << "Invalid image , failed to create cvmat for"
         " kHorizonVisionPixelFormatNone type image";
      cv_img->pixel_format =
          HorizonVisionPixelFormat::kHorizonVisionPixelFormatRawBGR;
      xstream_img->value = cv_img;
      break;
    }
    case kHorizonVisionPixelFormatRawBGR: {
      auto cv_img = std::make_shared<hobot::vision::CVImageFrame>();
      cv_img->img =
          cv::Mat(c_img.height, c_img.width, CV_8UC3, c_img.data);
//      cv::imwrite("c_bgr_in.jpg", cv_img->img);
      HOBOT_CHECK(!cv_img->img.empty())
      << "Invalid image , failed to create cvmat for"
         " kHorizonVisionPixelFormatRawBGR type image";
      cv_img->pixel_format =
          HorizonVisionPixelFormat::kHorizonVisionPixelFormatRawBGR;
      xstream_img->value = cv_img;
      break;
    }
#ifdef X2
    case kHorizonVisionPixelFormatX2SRC: {
      LOGF << "No support for kHorizonVisionPixelFormatX2SRC";
      break;
    }
    case kHorizonVisionPixelFormatX2PYM: {
      auto pym_img = std::make_shared<hobot::vision::PymImageFrame>();
      auto img_info = reinterpret_cast<img_info_t *>(c_img.data);
      HOBOT_CHECK(img_info);
      pym_img->img = *img_info;
      xstream_img->value = pym_img;
      check_pyramid(img_info);
      break;
    }
#endif
#ifdef X3
    case kHorizonVisionPixelFormatPYM: {
      auto pym_img = std::make_shared<hobot::vision::PymImageFrame>();
      hobot::vision::PymImageFrame *img_info =
        reinterpret_cast<hobot::vision::PymImageFrame *>(c_img.data);
      HOBOT_CHECK(img_info);
      check_pyramid(img_info);
      *pym_img = *img_info;  // default assign oper
      xstream_img->value = pym_img;
      break;
    }
#endif
    default: {
      LOGF << "No support image type " << image_type;
    }
  }
  return xstream_img;
}

XStreamImageFramePtr *ImageFrameConversion(
    const HorizonVisionImageFrame *c_img_frame) {
  auto xstream_img_frame = ImageConversion(c_img_frame->image);
  xstream_img_frame->value->time_stamp = c_img_frame->time_stamp;
  xstream_img_frame->value->channel_id = c_img_frame->channel_id;
  xstream_img_frame->value->frame_id = c_img_frame->frame_id;
  return xstream_img_frame;
}

}  // namespace util
}  // namespace vision
}  // namespace horizon


