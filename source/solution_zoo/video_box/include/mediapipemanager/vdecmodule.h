/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author:
 * @Mail: @horizon.ai
 */
#ifndef INCLUDE_VDECMODULE_H_
#define INCLUDE_VDECMODULE_H_

#include <mutex>
#include <vector>

#include "mediapipemanager/basicmediamoudle.h"

#include "hb_comm_vdec.h"

namespace horizon {
namespace vision {
class VdecModule : public BasicMediaModule {
public:
  VdecModule(/* args */);
  ~VdecModule();
  virtual int Init(uint32_t group_id,
                   const PipeModuleInfo *module_info) override;
  virtual int Start() override;
  virtual int Input(void *data) override;
  virtual int Output(void **data) override;
  virtual int OutputBufferFree(void *data) override;
  virtual int Stop() override;
  virtual int DeInit() override;

protected:
private:
  static std::once_flag flag_;
  uint32_t group_id_;
  uint32_t timeout_;
  // VIDEO_FRAME_S video_frame_;
  uint32_t frameDepth_;
  uint32_t buffer_index_;
  std::vector<VIDEO_FRAME_S> buffers_;
};

}  // namespace vision
}  // namespace horizon
#endif  // INCLUDE_VDECMODULE_H_