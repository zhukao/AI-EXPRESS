/***************************************************************************
* COPYRIGHT NOTICE
* Copyright 2016 Horizon Robotics, Inc.
* All rights reserved.
***************************************************************************/
/**
 * @file     mipi_host.h
 * @brief    MIPI HOST Common define
 * @author   tarryzhang (tianyu.zhang@hobot.cc)
 * @date     2017/7/6
 * @version  V1.0
 * @par      Horizon Robotics
 */
#ifndef __X2_MIPI_HOST_H__
#define __X2_MIPI_HOST_H__

#ifdef __cplusplus
extern "C"
{
#endif


#include <linux/types.h>

#define MIPIHOST_CHANNEL_NUM (2)
#define MIPIHOST_CHANNEL_0   (0)
#define MIPIHOST_CHANNEL_1   (1)

typedef struct _mipi_host_cfg_t {
	uint16_t  lane;
	uint16_t  datatype;
	uint16_t  fps;
	uint16_t  mclk;
	uint16_t  mipiclk;
	uint16_t  width;
	uint16_t  height;
	uint16_t  linelenth;
	uint16_t  framelenth;
	uint16_t  settle;
	uint16_t  hsaTime;
	uint16_t  hbpTime;
	uint16_t  hsdTime;
	uint16_t  channel_num;
	uint16_t  channel_sel[MIPIHOST_CHANNEL_NUM];
} mipi_host_cfg_t;

int hb_cam_mipi_host_start();
int hb_cam_mipi_host_stop();
int hb_cam_mipi_host_init(mipi_host_cfg_t *mipi_host_cfg);
int hb_cam_mipi_host_deinit();
int hb_cam_mipi_host_parser_config(void *root, mipi_host_cfg_t *cfg);

#ifdef __cplusplus
}
#endif


#endif /*__X2_MIPI_HOST_H__*/
