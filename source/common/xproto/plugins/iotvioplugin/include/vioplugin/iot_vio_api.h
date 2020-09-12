/***************************************************************************
 * COPYRIGHT NOTICE
 * Copyright 2019 Horizon Robotics, Inc.
 * All rights reserved.
 ***************************************************************************/
#ifndef INCLUDE_IOT_VIO_API_H_
#define INCLUDE_IOT_VIO_API_H_
#include <sys/types.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "hb_vio_interface.h"

/* for get info type */
#define IOT_VIO_SRC_INFO 1
#define IOT_VIO_PYM_INFO 2
#define IOT_VIO_SIF_INFO 3
#define IOT_VIO_IPU_STATE_INFO 4
#define IOT_VIO_FRAME_START_INFO 5
#define IOT_VIO_PYM_MULT_INFO 6
#define IOT_VIO_SRC_MULT_INFO 7
#define IOT_VIO_FEEDBACK_SRC_INFO 8
#define IOT_VIO_FEEDBACK_FLUSH 9
#define IOT_VIO_FEEDBACK_SRC_MULT_INFO 10
#define IOT_VIO_PYM_INFO_CONDITIONAL 11
#define IOT_VIO_FEEDBACK_PYM_PROCESS 12

/* max camera num */
#define CAM_MAX 4

typedef struct iot_mult_src_info_s {
  int img_num;
  hb_vio_buffer_t img_info[CAM_MAX];
} iot_mult_src_info_t;

typedef struct iot_mult_img_info_s {
  int img_num;
  pym_buffer_t img_info[CAM_MAX];
} iot_mult_img_info_t;

int iot_vio_free(void *data);
int iot_vio_free_info(uint32_t info_type, void *data);
int iot_vio_get_info(uint32_t info_type, void *data);
int iot_vio_set_info(u_int32_t info_type, void *data);
int iot_vio_stop(void);
int iot_vio_start(void);
int iot_vio_deinit();
int iot_vio_init(const char *cfg_file);
int iot_vio_pym_process(hb_vio_buffer_t *src_img_info);
int iot_vio_mult_pym_process(iot_mult_src_info_t *src_img_info);
int iot_cam_stop(uint32_t port);
int iot_cam_start(uint32_t port);
int iot_cam_deinit(uint32_t cfg_index);
int iot_cam_init(uint32_t cfg_index, const char *cfg_file);

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif  // INCLUDE_IOT_VIO_API_H_
