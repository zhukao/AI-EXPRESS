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

#include "hobotxsdk/xstream_data.h"
#include "horizon/vision_type/vision_type.h"
#include "xproto_msgtype/vioplugin_data.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace smartplugin {
using horizon::vision::xproto::basic_msgtype::VioMessage;

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
    int ref_count = 0;
  };
#endif

#ifdef X3
  struct InputFrameData {
    uint32_t image_num;
    std::vector<std::shared_ptr<hobot::vision::PymImageFrame>> img;
    void *context = nullptr;
    int ref_count = 0;
  };
#endif
  bool Reset();

  void PushFrame(const SmartInput *input);

  InputFrameData PopFrame(const int &source_id, const int &frame_id);

  // void OnXRocCallback(HobotXRoc::OutputDataPtr xroc_output);

 private:
  // <source id, frame id> => frame
  std::unordered_map<int32_t, std::unordered_map<int32_t, InputFrameData>>
      input_frames_;
  std::mutex map_mutex_;
};

}  // namespace smartplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif  // INCLUDE_SMARTPLUGIN_RUNTIME_MONITOR_H_
