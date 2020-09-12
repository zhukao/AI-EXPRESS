/***************************************************************************
* COPYRIGHT NOTICE
* Copyright 2016 Horizon Robotics, Inc.
* All rights reserved.
***************************************************************************/
/**
 * @file     mipi_dev.h
 * @brief    MIPI DEV Common define
 * @author   tarryzhang (tianyu.zhang@hobot.cc)
 * @date     2017/7/6
 * @version  V1.0
 * @par      Horizon Robotics
 */
#ifndef INC_HB_VIN_MIPI_DEV_H_
#define INC_HB_VIN_MIPI_DEV_H_

#ifdef __cplusplus
extern "C"
{
#endif


#include <linux/types.h>
#include "hb_vin_common.h"

int hb_vin_mipi_dev_start(entry_t *e);
int hb_vin_mipi_dev_stop(entry_t *e);
int hb_vin_mipi_dev_init(entry_t *e);
int hb_vin_mipi_dev_deinit(entry_t *e);
int hb_vin_mipi_dev_parser_config(void *root, entry_t *e);

#ifdef __cplusplus
}
#endif

#endif  // INC_HB_VIN_MIPI_DEV_H_
