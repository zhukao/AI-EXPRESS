/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.11.02
 */

#include <memory>
#include "utils_test.hpp"
#include "opencv2/opencv.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

ImageFramePtr crop_image(const ImageFramePtr &img_ptr,
                         const vision::BBox &crop_rect,
                         const uint32_t &output_width,
                         const uint32_t &output_height,
                         const bool &need_resize) {
  if (img_ptr->type == "CVImageFrame") {
    auto cv_image_frame = dynamic_cast<vision::CVImageFrame *>(img_ptr.get());
    cv::Rect roi(crop_rect.x1, crop_rect.y1, crop_rect.Width(),
                 crop_rect.Height());
    auto snapshot = std::make_shared<vision::CVImageFrame>();
    cv_image_frame->img(roi).copyTo(snapshot->img);
    snapshot->time_stamp = cv_image_frame->time_stamp;
    snapshot->frame_id = cv_image_frame->frame_id;
    snapshot->pixel_format = cv_image_frame->pixel_format;
    return snapshot;
  } else {
    return nullptr;
  }
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
