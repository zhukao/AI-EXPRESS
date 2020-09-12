/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2019-08-04 02:31:30
 * @Version: v0.0.1
 * @Brief: smart runtime monitor
 * @Note: simplify code from repo xperson global_config.h
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2019-09-29 05:03:14
 */

#ifndef INCLUDE_SMARTPLUGIN_RUNTIME_MONITOR_H_
#define INCLUDE_SMARTPLUGIN_RUNTIME_MONITOR_H_

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <map>

#include "hobotxsdk/xstream_data.h"
#include "horizon/vision_type/vision_type.h"
#include "xproto_msgtype/vioplugin_data.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace smartplugin_multiplebox {
using horizon::vision::xproto::basic_msgtype::VioMessage;
using Time_Point = std::chrono::time_point<std::chrono::system_clock>;

struct SmartInput {
  std::shared_ptr<VioMessage> frame_info;
  void *context;
};

class RuntimeMonitor {
public:
  RuntimeMonitor();
#ifdef X2
  struct InputFrameData {
    uint32_t image_num;
    HorizonVisionImageFrame **img;
    void *context = nullptr;
  };
#endif

#ifdef X3
  struct InputFrameData {
    uint32_t image_num;
    std::vector<std::shared_ptr<hobot::vision::PymImageFrame>> img;
    void *context = nullptr;
  };
#endif
  bool Reset();

  void PushFrame(const SmartInput *input);

  InputFrameData PopFrame(const uint64_t &frame_id, int channel_id = -1);

  uint64_t GetFrontFrame(uint8_t channel_id);

  void FrameStatistic(int channel);
  // void OnXStreamCallback(xstream::OutputDataPtr xstream_output);

private:
  std::unordered_map<int32_t, InputFrameData> input_frames_;
  std::mutex map_mutex_;
  std::vector<std::shared_ptr<Time_Point>> TP;
  const int channel_ = 4;
  const uint8_t channel_frame_queue_limit_ = 50;
  std::vector<std::map<uint64_t, bool>> channel_frame_id_;
};

}  // namespace smartplugin_multiplebox
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif  //  INCLUDE_SMARTPLUGIN_RUNTIME_MONITOR_H_
