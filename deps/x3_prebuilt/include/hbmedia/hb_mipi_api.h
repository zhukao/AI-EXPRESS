/***************************************************************************
* COPYRIGHT NOTICE
* Copyright 2020 Horizon Robotics, Inc.
* All rights reserved.
***************************************************************************/
#ifndef MIPI_HB_MIPI_API_H_
#define MIPI_HB_MIPI_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#define MIPIHOST_CHANNEL_NUM (4)

enum HB_MIPI_ERROR_CODE {
	MIPI_INPUT_PARAM_INVALID = 30,
	MIPI_SENSOR_INIT_FAIL,
	MIPI_SENSOR_DEINIT_FAIL,
	MIPI_RESET_SENSOR_FAIL,
	MIPI_UNRESET_SENSOR_FAIL,
	MIPI_RESET_FAIL,
	MIPI_UNRESET_FAIL,
	MIPI_SETDEV_ATTR_FAIL,
	MIPI_CLEAR_FAIL,
	MIPI_SET_BYPASS_FAIL,
	MIPI_ENABLE_SENSOR_CLK_FAIL,
	MIPI_DISENABLE_SENSOR_CLK_FAIL,
	MIPI_SENSOR_FUNC_NOT_SUPPORT,
	MIPI_SET_SENSOR_FPS_FAIL,
	MIPI_I2C_WRITE_FAIL,
	MIPI_I2C_READ_FAIL,
	MIPI_SW_SENSOR_MODE_FAIL,
	MIPI_INVALID_OPERATION
};

typedef struct HB_MIPI_SPI_DATA_S {
	int spi_mode;
	int spi_cs;
	uint32_t spi_speed;
}MIPI_SPI_DATA_S;

typedef struct HB_MIPI_DESERIAL_INFO_T {
	int bus_type;
	int bus_num;
	int deserial_addr;
	char *deserial_name;
}MIPI_DESERIAL_INFO_T;

typedef enum HB_MIPI_INPUT_MODE_E {
	INPUT_MODE_MIPI         = 0x0,              /* mipi */
	INPUT_MODE_DVP          = 0x1,              /* DVP*/
	INPUT_MODE_BUTT
} MIPI_INPUT_MODE_E;

typedef enum HB_MIPI_SENSOR_MODE_E {
	NORMAL_M = 1,
	DOL2_M,
	DOL3_M,
	DOL4_M,
	PWL_M,
	INVAILD_MODE,
}MIPI_SENSOR_MODE_E;

typedef struct HB_MIPI_SNS_INFO_S {
	int port;
	int dev_port;
	int bus_type;
	int bus_num;
	int fps;
	int resolution;
	int sensor_addr;
	int serial_addr;
	int entry_index;
	MIPI_SENSOR_MODE_E sensor_mode;
	int reg_width;
	char *sensor_name;
	int extra_mode;
	int deserial_index;
	int deserial_port;
	MIPI_SPI_DATA_S spi_info;
}MIPI_SNS_INFO_S;

typedef struct HB_MIPI_SENSOR_INFO_S {
	int  	 deseEnable;
	MIPI_INPUT_MODE_E inputMode;
	MIPI_DESERIAL_INFO_T deserialInfo;
	MIPI_SNS_INFO_S sensorInfo;
}MIPI_SENSOR_INFO_S;

typedef struct HB_MIPI_DEV_CFG_S {
	uint32_t  enable;
} MIPI_DEV_CFG_S;

typedef struct HB_MIPI_HOST_CFG_S {
	uint16_t  lane;
	uint16_t  datatype;
	uint16_t  mclk;
	uint16_t  mipiclk;
	uint16_t  fps;
	uint16_t  width;
	uint16_t  height;
	uint16_t  linelenth;
	uint16_t  framelenth;
	uint16_t  settle;
	uint16_t  channel_num;
	uint16_t  channel_sel[MIPIHOST_CHANNEL_NUM];
} MIPI_HOST_CFG_S;

typedef struct HB_MIPI_ATTR_S {
	MIPI_HOST_CFG_S mipi_host_cfg;
	uint32_t  dev_enable;
} MIPI_ATTR_S;

extern int HB_MIPI_SetBus(MIPI_SENSOR_INFO_S *snsInfo, uint32_t busNum);
extern int HB_MIPI_SetPort(MIPI_SENSOR_INFO_S *snsInfo, uint32_t port);
extern int HB_MIPI_SensorBindSerdes(MIPI_SENSOR_INFO_S *snsInfo,
	uint32_t serdesIdx, uint32_t serdesPort);
extern int HB_MIPI_SensorBindMipi(MIPI_SENSOR_INFO_S *snsInfo, uint32_t mipiIdx);
extern int HB_MIPI_SetExtraMode(MIPI_SENSOR_INFO_S *snsInfo, uint32_t ExtraMode);
extern int HB_MIPI_InitSensor(uint32_t devId, MIPI_SENSOR_INFO_S *snsInfo);
// extern int HB_MIPI_MipiBindSerdes(uint32_t mipiIdx, uint32_t serdesId);
extern int HB_MIPI_DeinitSensor(uint32_t devId);
extern int HB_MIPI_ResetSensor(uint32_t devId);
extern int HB_MIPI_UnresetSensor(uint32_t devId);
extern int HB_MIPI_SetSensorFps(uint32_t devId, uint32_t fps);
extern int HB_MIPI_GetSensorInfo(uint32_t devId, MIPI_SENSOR_INFO_S *snsInfo);
extern int HB_MIPI_SwSensorFps(uint32_t devId, uint32_t fps);
extern int HB_MIPI_SwSensorMode(uint32_t devId, uint32_t SenMode);

extern int HB_MIPI_ReadSensor(uint32_t devId,
	uint32_t regAddr, char *buffer, uint32_t size);
extern int HB_MIPI_WriteSensor(uint32_t devId,
	uint32_t regAddr, char *buffer, uint32_t size);

extern int HB_MIPI_SetMipiHsMode(uint32_t mipiIdx, uint32_t laneMode);
extern int HB_MIPI_ResetMipi(uint32_t mipiIdx);
extern int HB_MIPI_UnresetMipi(uint32_t mipiIdx);
extern int HB_MIPI_EnableSensorClock(uint32_t mipiIdx);
extern int HB_MIPI_DisableSensorClock(uint32_t mipiIdx);
extern int HB_MIPI_SetMipiAttr(uint32_t mipiIdx, MIPI_ATTR_S *mipiAttr);
extern int HB_MIPI_Clear(uint32_t mipiIdx);
// extern int HB_MIPI_EnableTxDev(uint32_t  mipiIdx);

#ifdef __cplusplus
}
#endif

#endif  // MIPI_HB_MIPI_API_H_
