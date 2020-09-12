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
#ifndef INC_HB_VIN_MIPI_HOST_H_
#define INC_HB_VIN_MIPI_HOST_H_

#ifdef __cplusplus
extern "C"
{
#endif


#include <linux/types.h>
#include "hb_vin_common.h"

int hb_vin_mipi_host_start(entry_t *e);
int hb_vin_mipi_host_stop(entry_t *e);
int hb_vin_mipi_host_init(entry_t *e);
int hb_vin_mipi_host_deinit(entry_t *e);
int hb_vin_mipi_host_parser_config(void *root, entry_t *e);
int hb_vin_mipi_host_snrclk_set_en(entry_t *e, uint32_t enable);
int hb_vin_mipi_host_snrclk_set_freq(entry_t *e, uint32_t freq);
int hb_vin_mipi_host_pre_init_request(entry_t *e, uint32_t timeout);
int hb_vin_mipi_host_pre_start_request(entry_t *e, uint32_t timeout);
int hb_vin_mipi_host_pre_init_result(entry_t *e, uint32_t result);
int hb_vin_mipi_host_pre_start_result(entry_t *e, uint32_t result);

#ifdef __cplusplus
}
#endif

#endif  // INC_HB_VIN_MIPI_HOST_H_
