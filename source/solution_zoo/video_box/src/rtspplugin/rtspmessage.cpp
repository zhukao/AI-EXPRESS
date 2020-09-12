/*
 * @Description: implement of vioplugin
 * @Author: fei.cheng@horizon.ai
 * @Date: 2019-08-26 16:17:25
 * @Author: songshan.gong@horizon.ai
 * @Date: 2019-09-26 16:17:25
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-10-16 15:34:58
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */
#include "rtspplugin/rtspmessage.h"
#include "hobotlog/hobotlog.hpp"
#include "horizon/vision_type/vision_type_util.h"
#include "xproto_msgtype/protobuf/x2.pb.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace horizon {
namespace vision {
namespace xproto {
namespace rtspplugin {

ImageVioMessage::ImageVioMessage(
    std::vector<std::shared_ptr<PymImageFrame>> &image_frame, uint32_t img_num,
    bool is_valid, int channel, std::shared_ptr<MediaPipeLine> pipeline,
    void *data) {
  type_ = TYPE_IMAGE_MESSAGE;
  num_ = img_num;
  channel_ = channel;
  if (image_frame.size() > 0) {
    time_stamp_ = image_frame[0]->time_stamp;
    sequence_id_ = image_frame[0]->frame_id;
    image_.resize(image_frame.size());
  }
  is_valid_uri_ = is_valid;
  for (uint32_t i = 0; i < image_frame.size(); ++i) {
    image_[i] = image_frame[i];
  }
  pipeline_ = pipeline;
  slot_data_ = data;
}

ImageVioMessage::~ImageVioMessage() { LOGI << "call ~ImageVioMessage"; }

DropVioMessage::DropVioMessage(uint64_t timestamp, uint64_t seq_id) {
  type_ = TYPE_DROP_MESSAGE;
  time_stamp_ = timestamp;
  sequence_id_ = seq_id;
}

std::string DropVioMessage::Serialize() {
  std::string smart_str;
  x2::SmartFrameMessage smart_frame_message;

  LOGI << "Drop Serialize";
  smart_frame_message.set_timestamp_(time_stamp_);
  smart_frame_message.set_error_code_(1);
  smart_frame_message.SerializeToString(&smart_str);

  return smart_str;
}

void ImageVioMessage::FreeImage(int tmp) {
  //   if (image_.size() > 0) {
  //     // free image
  //     if (num_ == 1) {
  //       LOGI << "begin remove one vio slot";
  // #if defined(X3_X2_VIO) || defined(X3_IOT_VIO)
  //       VioFeedbackContext *feedback_context =
  //           reinterpret_cast<VioFeedbackContext *>(image_[0]->context);
  //       if (hb_vio_free_info(HB_VIO_SRC_INFO,
  //                            &(feedback_context->src_info)) < 0) {
  //         LOGE << "hb_vio_free_info failed";
  //       }
  //       int ret = hb_vio_free(&(feedback_context->pym_img_info));
  //       if (ret != 0) {
  //         LOGE << "hb_vio_free failed";
  //       }
  //       free(feedback_context);
  //       LOGD << "free feedback context success";
  // #endif
  //       image_[0]->context = nullptr;
  //       image_[0] = nullptr;
  //     } else if (num_ == 2) {
  //       // todo
  //     }
  //     image_.clear();
  //   }
}

void ImageVioMessage::FreeImage() {
  //   if (image_.size() > 0) {
  //     // free image
  //     if (num_ == 1) {
  //       LOGI << "begin remove one vio slot";
  // #ifdef X3_X2_VIO
  //       img_info_t *img_info = reinterpret_cast<img_info_t
  //       *>(image_[0]->context); hb_vio_free(img_info); free(img_info);
  // #elif defined(X3_IOT_VIO)
  //       pym_buffer_t *img_info =
  //           reinterpret_cast<pym_buffer_t *>(image_[0]->context);
  //       iot_vio_free(img_info);
  //       free(img_info);
  // #endif
  //       image_[0]->context = nullptr;
  //       image_[0] = nullptr;
  //     } else if (num_ == 2) {
  //       // todo
  //     }
  //     image_.clear();
  //   }
}

}  // namespace rtspplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
