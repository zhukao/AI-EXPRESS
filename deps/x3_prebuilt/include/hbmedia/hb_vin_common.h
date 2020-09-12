/***************************************************************************
* COPYRIGHT NOTICE
* Copyright 2019 Horizon Robotics, Inc.
* All rights reserved.
***************************************************************************/
#ifndef INC_HB_VIN_COMMON_H_
#define INC_HB_VIN_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define HB_VIN_MIPI_HOST_START_FAIL		500
#define HB_VIN_MIPI_HOST_STOP_FAIL		501
#define HB_VIN_MIPI_HOST_INIT_FAIL		502
#define HB_VIN_MIPI_HOST_PARSER_FAIL	503
#define HB_VIN_MIPI_HOST_NOT_ENABLE		504
#define HB_VIN_MIPI_HOST_SNRCLK_SET_EN_FAIL   505
#define HB_VIN_MIPI_HOST_SNRCLK_SET_FREQ_FAIL 506
#define HB_VIN_MIPI_HOST_PPE_INIT_REQUEST_FAIL  507
#define HB_VIN_MIPI_HOST_PRE_START_REQUEST_FAIL 508
#define HB_VIN_MIPI_HOST_PRE_INIT_RESULT_FAIL   509
#define HB_VIN_MIPI_HOST_PRE_START_RESULT_FAIL  510
#define HB_VIN_MIPI_DEV_START_FAIL		600
#define HB_VIN_MIPI_DEV_STOP_FAIL		601
#define HB_VIN_MIPI_DEV_INIT_FAIL		602
#define HB_VIN_MIPI_DEV_PARSER_FAIL		603
#define HB_VIN_MIPI_DEV_NOT_ENABLE		604

#ifndef ENTRY_NUM
#define ENTRY_NUM 4
#endif
#ifndef RET_OK
#define RET_OK 0
#endif
#ifndef RET_ERROR
#define RET_ERROR 1
#endif

#define HB_VIN_MIPI_HOST_PATH	"/dev/mipi_host"
#define HB_VIN_MIPI_DEV_PATH	"/dev/mipi_dev"

#define MIPIDEV_CHANNEL_NUM  (4)
#define MIPIDEV_CHANNEL_0    (0)
#define MIPIDEV_CHANNEL_1    (1)
#define MIPIDEV_CHANNEL_2    (2)
#define MIPIDEV_CHANNEL_3    (3)

typedef struct _mipi_dev_cfg_t {
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
	uint16_t  vpg;
	uint16_t  ipi_lines;
	uint16_t  channel_num;
	uint16_t  channel_sel[MIPIDEV_CHANNEL_NUM];
} mipi_dev_cfg_t;

#define MIPIHOST_CHANNEL_NUM (4)
#define MIPIHOST_CHANNEL_0   (0)
#define MIPIHOST_CHANNEL_1   (1)
#define MIPIHOST_CHANNEL_2   (2)
#define MIPIHOST_CHANNEL_3   (3)

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

#define MIPI_PARAM_NAME_LEN	(32)
#define MIPI_PARAM_MAX		(10)
#define MIPI_SYSFS_PATH_PRE		"/sys/class/vps"
#define MIPI_SYSFS_DIR_PARAM	"param"

typedef struct _mipi_param_t {
	char name[MIPI_PARAM_NAME_LEN];
	int32_t value;
} mipi_param_t;

typedef struct entry_s {
	mipi_host_cfg_t mipi_host_cfg;
	mipi_dev_cfg_t  mipi_dev_cfg;
	int entry_num;
	int host_fd;
	int dev_fd;
	int host_enable;
	int dev_enable;
	int init_state;
	int start_state;
	char host_path[128];
	char dev_path[128];
	mipi_param_t host_params[MIPI_PARAM_MAX];
	mipi_param_t dev_params[MIPI_PARAM_MAX];
}entry_t;

#ifndef NOSIF
#define HB_VIN_SIF_PATH	"/dev/sif_capture"

#define SIF_IOC_MAGIC 'x'
#define SIF_IOC_BYPASS _IOW(SIF_IOC_MAGIC, 7, int)

#define HB_VIN_SIF_OPEN_DEV_FAIL	700
#define HB_VIN_SIF_BYPASS_FAIL		717

typedef struct sif_input_bypass {
	uint32_t enable_bypass;
	uint32_t enable_frame_id;
	uint32_t init_frame_id;
	uint32_t set_bypass_channels;
} sif_input_bypass_t;
#endif

enum VIN_STATUS{
	VIN_DEINIT,
	VIN_INIT,
	VIN_START,
	VIN_STOP
};

#ifdef __cplusplus
}
#endif

#endif  // INC_HB_VIN_COMMON_H_
