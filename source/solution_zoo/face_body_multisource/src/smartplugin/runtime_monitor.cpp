/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2019-08-04 03:07:26
 * @Version: v0.0.1
 * @Brief: runtime monitor implementation
 * @Note:  extracted from repo xperson's global_config.cpp
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2019-08-22 23:46:17
 */

#include "smartplugin/runtime_monitor.h"
#include <memory>
#include <mutex>
#include "hobotlog/hobotlog.hpp"
#include "horizon/vision/util.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace smartplugin {
using ImageFramePtr = std::shared_ptr<hobot::vision::ImageFrame>;
using XStreamImageFramePtr = xstream::XStreamData<ImageFramePtr>;

void RuntimeMonitor::PushFrame(const SmartInput *input) {
  std::unique_lock<std::mutex> lock(map_mutex_);
  HOBOT_CHECK(input) << "Null HorizonVisionImageFrame";
  auto frame_info = input->frame_info;
  HOBOT_CHECK(frame_info->num_ > 0);
#ifdef X2
  auto image0 = frame_info->image_[0];
  uint64_t frame_id = image0->frame_id;
  uint32_t channel_id = image0->channel_id;
#endif
#ifdef X3
  std::shared_ptr<hobot::vision::PymImageFrame> image0 = frame_info->image_[0];
  uint64_t frame_id = image0->frame_id;
  uint32_t channel_id = image0->channel_id;
#endif
  // 查找source id对应的那一组输入的frame
  auto &frame_set_ = input_frames_[channel_id];

  LOGI << "Push source id: " << channel_id
    << ", frame id: " << frame_id;

  if (frame_set_[frame_id].ref_count == 0) {
    LOGI << "Insert frame id:" << frame_id;
    frame_set_[frame_id].image_num = frame_info->num_;
    frame_set_[frame_id].img = frame_info->image_;
    frame_set_[frame_id].context = input->context;  // input->context = input
  } else {
    delete input;
  }

  frame_set_[frame_id].ref_count++;
}

RuntimeMonitor::InputFrameData RuntimeMonitor::PopFrame(
    const int32_t &source_id, const int32_t &frame_id) {
  std::unique_lock<std::mutex> lock(map_mutex_);
  InputFrameData input;

  LOGI << "Pop frame source:" << source_id
    << ", frame id:" << frame_id;

  auto frame_set_itr = input_frames_.find(source_id);
  if (frame_set_itr == input_frames_.end()) {
    LOGW << "Unknow source id: " << source_id;
    return input;
  }
  auto &frame_set_ = frame_set_itr->second;

  auto frame_iter = frame_set_.find(frame_id);
  if (frame_iter == frame_set_.end()) {
    LOGW << "Unknow source id: " << source_id
      << ", frame id: " << frame_id;
    return input;
  }
  auto &origin_input = frame_iter->second;

  if ((--origin_input.ref_count) == 0) {
    LOGI << "erase frame id:" << frame_id;
    frame_set_.erase(frame_iter);
  }
  return origin_input;
}

RuntimeMonitor::RuntimeMonitor() { Reset(); }

bool RuntimeMonitor::Reset() { return true; }

}  // namespace smartplugin
}  // namespace xproto
}  //  namespace vision
}  //  namespace horizon
