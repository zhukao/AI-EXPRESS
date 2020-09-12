/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2019-08-02 04:05:52
 * @Version: v0.0.1
 * @Brief: implemenation of converter.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2019-08-02 06:26:01
 */
#include <turbojpeg.h>
#include "hobotxsdk/xstream_data.h"
#include "horizon/vision/util.h"

#include "hobotlog/hobotlog.hpp"
#include "smartplugin/convert.h"
#include "xproto_msgtype/vioplugin_data.h"
using ImageFramePtr = std::shared_ptr<hobot::vision::ImageFrame>;
namespace horizon {
namespace iot {

using horizon::vision::util::ImageFrameConversion;
int Convertor::image_compress_quality = 50;

xstream::InputDataPtr Convertor::ConvertInput(const VioMessage *input) {
  xstream::InputDataPtr inputdata(new xstream::InputData());
  HOBOT_CHECK(input != nullptr && input->num_ > 0 && input->is_valid_uri_);

  // \todo need a better way to identify mono or semi cameras
  for (uint32_t image_index = 0; image_index < 1; ++image_index) {
    xstream::BaseDataPtr xstream_input_data;
    if (input->num_ > image_index) {
#ifdef X2
      auto xstream_img = ImageFrameConversion(input->image_[image_index]);
      xstream_input_data = xstream::BaseDataPtr(xstream_img);
      LOGI << "Input Frame ID = " << xstream_img->value->frame_id
           << ", Timestamp = " << xstream_img->value->time_stamp;
#endif

#ifdef X3
      std::shared_ptr<hobot::vision::PymImageFrame> pym_img =
          input->image_[image_index];
      LOGI << "vio message, frame_id = " << pym_img->frame_id;
      for (uint32_t i = 0; i < DOWN_SCALE_MAX; ++i) {
        LOGD << "vio message, pym_level_" << i
        << ", width=" << pym_img->down_scale[i].width
        << ", height=" << pym_img->down_scale[i].height
        << ", stride=" << pym_img->down_scale[i].stride;
      }
      auto xstream_img =
          std::make_shared<xstream::XStreamData<ImageFramePtr>>();
      xstream_img->type_ = "ImageFrame";
      xstream_img->value =
          std::static_pointer_cast<hobot::vision::ImageFrame>(pym_img);
      LOGI << "Input Frame ID = " << xstream_img->value->frame_id
           << ", Timestamp = " << xstream_img->value->time_stamp;
      xstream_input_data = xstream::BaseDataPtr(xstream_img);
#endif
    } else {
      xstream_input_data = std::make_shared<xstream::BaseData>();
      xstream_input_data->state_ = xstream::DataState::INVALID;
    }

    if (image_index == uint32_t{0}) {
      if (input->num_ == 1) {
        xstream_input_data->name_ = "image";
      } else {
        xstream_input_data->name_ = "rgb_image";
      }
    } else {
      xstream_input_data->name_ = "nir_image";
    }
    LOGI << "input name:" << xstream_input_data->name_;
    inputdata->datas_.emplace_back(xstream_input_data);
  }

  return inputdata;
}

}  // namespace iot
}  // namespace horizon
