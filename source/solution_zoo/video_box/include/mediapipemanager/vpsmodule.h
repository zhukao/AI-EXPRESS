/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author:
 * @Mail: @horizon.ai
 */
#ifndef INCLUDE_VPSMODULE_H_
#define INCLUDE_VPSMODULE_H_

#include <vector>

#include "hb_vio_interface.h"
#include "mediapipemanager/basicmediamoudle.h"

namespace horizon {
namespace vision {
class VpsModule : public BasicMediaModule {
public:
  VpsModule(/* args */);
  ~VpsModule();
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
  uint32_t group_id_;
  uint32_t timeout_;
  uint32_t frameDepth_;
  uint32_t buffer_index_;
  std::vector<pym_buffer_t> buffers_;
};

}  // namespace vision
}  // namespace horizon
#endif  // INCLUDE_VPSMODULE_H_