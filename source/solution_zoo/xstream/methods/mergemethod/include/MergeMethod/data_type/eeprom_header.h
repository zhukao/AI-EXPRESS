/*
 * @Description: fake implementation of eeprom
 * @Author: yutong.pan@horizon.ai
 * @Date: 2020-01-15
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */

#ifndef INCLUDE_MERGEMETHOD_DATA_TYPE_EEPROM_HEADER_H_
#define INCLUDE_MERGEMETHOD_DATA_TYPE_EEPROM_HEADER_H_

namespace xstream {

#define CAM_CALIB_START_ADDR (0x0)
#define CAM_CALIB_DATA_LENGTH (72)

#define CAMERA_EVENT_EEPROM_CALIBRATION (1UL << 1)
typedef struct camera_calib_eeprom_s {
#define EEPROM_RD_MODE 0
#define EEPROM_WR_MODE 1
  int wr_mode;  // 0 : for read, 1 for write
  int addr;
  int length;
  char *buff;
} calib_eeprom_t;

inline int hb_cam_process(int event, void *arg) { return -1; }
}  // namespace xstream

#endif  // INCLUDE_MERGEMETHOD_DATA_TYPE_EEPROM_HEADER_H_
