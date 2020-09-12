/***************************************************************************
* COPYRIGHT NOTICE
* Copyright 2019 Horizon Robotics, Inc.
* All rights reserved.
***************************************************************************/
#ifndef INC_HB_VIN_H_
#define INC_HB_VIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

extern int hb_cam_mipi_parse_cfg(char *filename, int fps, int resolution, int entry_num);
extern int hb_vin_init(uint32_t entry_num);
extern int hb_vin_stop(uint32_t entry_num);
extern int hb_vin_start(uint32_t entry_num);
extern int hb_vin_deinit(uint32_t entry_num);
extern int hb_vin_reset(uint32_t entry_num);
extern int hb_vin_snrclk_set_en(uint32_t entry_num, uint32_t enable);
extern int hb_vin_snrclk_set_freq(uint32_t entry_num, uint32_t freq);
extern int hb_vin_chn_bypass(uint32_t port, uint32_t enable, uint32_t mux_sel, uint32_t chn_mask);
extern int hb_vin_iar_bypass(uint32_t port, uint32_t enable, uint32_t enable_frame_id, uint32_t init_frame_id);
extern int hb_vin_set_bypass(uint32_t port, uint32_t enable);
extern int hb_vin_pre_request(uint32_t entry_num, uint32_t type, uint32_t timeout);
extern int hb_vin_pre_result(uint32_t entry_num, uint32_t type, uint32_t result);

#ifdef __cplusplus
}
#endif

#endif  // INC_HB_VIN_H_
