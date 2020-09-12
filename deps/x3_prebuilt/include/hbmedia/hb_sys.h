/***************************************************************************
* COPYRIGHT NOTICE
* Copyright 2019 Horizon Robotics, Inc.
* All rights reserved.
***************************************************************************/
#ifndef __HB_SYS_H__
#define __HB_SYS_H__

#include "hb_mode.h"

typedef enum HB_SYS_MOD_ID_E {
	HB_ID_SYS = 0,
	HB_ID_VIN,
	HB_ID_VOT,
	HB_ID_VPS,
	HB_ID_RGN,
	HB_ID_AIN,
	HB_ID_AOT,
	HB_ID_VENC,
	HB_ID_VDEC,
	HB_ID_AENC,
	HB_ID_ADEC,
} SYS_MOD_ID_E;

typedef struct HB_SYS_MOD_S {
	SYS_MOD_ID_E enModId;
	uint8_t s32DevId;
	uint8_t s32ChnId;
} SYS_MOD_S;

extern int HB_SYS_Init(void);

extern int HB_SYS_Exit(void);

extern int HB_SYS_Bind(const SYS_MOD_S *pstSrcMod, const SYS_MOD_S *pstDstMod);

extern int HB_SYS_UnBind(const SYS_MOD_S *pstSrcMod, const SYS_MOD_S *pstDstMod);

extern int HB_SYS_SetVINVPSMode(int pipeId, const SYS_VIN_VPS_MODE_E mode);

extern int HB_SYS_GetVINVPSMode(int pipeId);

extern int HB_SYS_Bind_Layer(const SYS_MOD_S *pstSrcMod,
		const SYS_MOD_S *pstDstMod);

extern int HB_SYS_UnBind_Layer(const SYS_MOD_S *pstSrcMod,
		const SYS_MOD_S *pstDstMod);



#endif
