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
#include "smartplugin/utils/time_helper.h"
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
#endif
#ifdef X3
  std::shared_ptr<hobot::vision::PymImageFrame> image0 = frame_info->image_[0];
  uint64_t frame_id = image0->frame_id;
  LOGI << "PushFrame frame_id = " << frame_id << std::endl;
#endif
  input_frames_[frame_id].image_num = frame_info->num_;
  input_frames_[frame_id].img = frame_info->image_;
  input_frames_[frame_id].context = input->context;
  if (frame_info->profile_) {
    input_frames_[frame_id].vio_msg = frame_info;
  } else {
    input_frames_[frame_id].vio_msg = nullptr;
  }
}

RuntimeMonitor::InputFrameData RuntimeMonitor::PopFrame(
    const int32_t &frame_id) {
  std::unique_lock<std::mutex> lock(map_mutex_);
  InputFrameData input;
  auto itr = input_frames_.find(frame_id);
  if (itr != input_frames_.end()) {
    input = itr->second;
    LOGI << "Pop frame " << frame_id;
    input_frames_.erase(itr);
  }
  return input;
}

void RuntimeMonitor::FrameStatistic() {
    // 实际智能帧率计算
  static int fps = 0;
  // 耗时统计，ms
  static auto lastTime = hobot::Timer::tic();
  static int frameCount = 0;

  ++frameCount;

  auto curTime = hobot::Timer::toc(lastTime);
  // 统计数据发送帧率
  if (curTime > 1000) {
    fps = frameCount;
    frameCount = 0;
    lastTime = hobot::Timer::tic();
    LOGW << "Smart fps = " << fps;
    frame_fps_ = fps;
  }
}

RuntimeMonitor::RuntimeMonitor() { Reset(); }

bool RuntimeMonitor::Reset() { return true; }

}  // namespace smartplugin
}  // namespace xproto
}  //  namespace vision
}  //  namespace horizon
