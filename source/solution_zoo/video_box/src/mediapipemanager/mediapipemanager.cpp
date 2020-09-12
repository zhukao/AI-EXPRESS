/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author:
 * @Mail: @horizon.ai
 */
#include "mediapipemanager/mediapipemanager.h"

#include <memory>
#include <mutex>

#include "hb_sys.h"
#include "hb_vp_api.h"
#include "hobotlog/hobotlog.hpp"

namespace horizon {
namespace vision {
MediaPipeManager *MediaPipeManager::instance_ = NULL;

MediaPipeManager::MediaPipeManager()
    : initialized_(false), vp_max_pool_count_(32) {
  ;
}

MediaPipeManager &MediaPipeManager::GetInstance() {
  static std::once_flag flag;
  std::call_once(flag, [&]() { instance_ = new MediaPipeManager(); });
  return *instance_;
}

int MediaPipeManager::Init(int max_pool_count) {
  std::lock_guard<std::mutex> lock(manager_mutex_);
  int ret = 0;
  if (initialized_) {
    LOGW << "MediaPipeManager already initialized";
    return 0;
  } else {
    initialized_ = true;
    ret = HB_SYS_Init();
    if (ret != 0) {
      LOGF << "HB_SYS_Init Failed! ret = " << ret;
      return ret;
    }
    VP_CONFIG_S struVpConf;
    memset(&struVpConf, 0x00, sizeof(VP_CONFIG_S));
    struVpConf.u32MaxPoolCnt = max_pool_count;
    ret = HB_VP_SetConfig(&struVpConf);
    if (ret != 0) {
      LOGF << "HB_VP_SetConfig Failed! ret = " << ret;
      return ret;
    }
    ret = HB_VP_Init();
    if (ret != 0) {
      LOGF << "HB_VP_Init Failed. ret = " << ret;
      return ret;
    }
  }
  return ret;
}

int MediaPipeManager::AddPipeLine(std::shared_ptr<MediaPipeLine> pipeline) {
  LOGE << "MediaPipeManager. add pipeline ";
  media_pipelines_.push_back(pipeline);
  return 0;
}

const std::vector<std::shared_ptr<MediaPipeLine>> &
MediaPipeManager::GetPipeLine() {
  return media_pipelines_;
}
}  // namespace vision
}  // namespace horizon