/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @brief     used to support x3 bpu-predict old interface
 * @author    hangjun.yang
 * @date      2020.05.19
 */

#ifndef BPU_PREDICT_X3_H_
#define BPU_PREDICT_X3_H_

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "hb_vio_interface.h"

namespace bpu_predict_x3 {

#define DOWN_SCALE_MAIN_MAX 6
#define DOWN_SCALE_MAX 24
#define UP_SCALE_MAX 6

typedef struct x2_addr_info_s {
  uint16_t width;
  uint16_t height;
  uint16_t step;
  uint64_t y_paddr;
  uint64_t c_paddr;
  uint64_t y_vaddr;
  uint64_t c_vaddr;
} x2_addr_info_t;

typedef struct src_img_info_s {
  int cam_id;
  int slot_id;
  int img_format;
  int frame_id;
  int64_t timestamp;
  x2_addr_info_t src_img;
  x2_addr_info_t scaler_img;
} src_img_info_t;

typedef struct img_info_s {
  int slot_id;
  int frame_id;
  int64_t timestamp;
  int img_format;
  int ds_pym_layer;
  int us_pym_layer;
  x2_addr_info_t down_scale[DOWN_SCALE_MAX];
  x2_addr_info_t up_scale[UP_SCALE_MAX];
  x2_addr_info_t down_scale_main[DOWN_SCALE_MAIN_MAX];
  int cam_id;
  uint16_t pipeline_id;
  hb_vio_buffer_t src_img_info;
  pym_buffer_t img_info;
} img_info_t;

struct PyramidResult {
  img_info_t result_info;
  bool valid;
};
}  // namespace bpu_predict_x3

#endif // BPU_PREDICT_X3_H_

