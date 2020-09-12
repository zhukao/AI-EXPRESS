#ifndef INCLUDE_BASICMEDIAMODULE_H_
#define INCLUDE_BASICMEDIAMODULE_H_

#include <cstdint>
#include <cstring>

#include "hb_vio_interface.h"

namespace horizon {
namespace vision {

struct PipeModuleInfo {
  void *attr;
  uint32_t input_width;
  uint32_t input_height;
  uint32_t output_width;
  uint32_t output_height;
  uint32_t frame_depth;
};

class BasicMediaModule {
public:
  virtual int Init(uint32_t group_id, const PipeModuleInfo *module_info) = 0;
  virtual int Start() = 0;
  virtual int Input(void *data) = 0;
  virtual int Output(void **data) = 0;
  virtual int OutputBufferFree(void *data) = 0;
  virtual int Stop() = 0;
  virtual int DeInit() = 0;
  BasicMediaModule(){};
  ~BasicMediaModule(){};

protected:
private:
};

}  // namespace vision
}  // namespace horizon

#endif  // INCLUDE_BASICMEDIAMODULE_H_