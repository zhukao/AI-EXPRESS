/*****************************************************************************
 * Copyright (c) 2019 Horizon Robotics.
 * Description: 定义HAPI 视频输出模块结构体
 * Author: xiaolin.huang
 * Version:
 * Date: 2019-12-12 14:27:20
 * LastEditTime: 2020-01-09 10:55:54
 * History:
 *****************************************************************************/

#ifndef __HB_COMM_VOT_H__
#define __HB_COMM_VOT_H__

#include <stdint.h>
#include "hb_type.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

// #define HB_SUCCESS 0
// #define HB_FAILURE -1

/* System define error code */
/* 资源忙 */
#define HB_ERR_VOT_BUSY 0xA401
/* 内存不足 */
#define HB_ERR_VOT_NO_MEM 0xA402
/* 函数参数中有空指针 */
#define HB_ERR_VOT_NULL_PTR 0xA403
/* 系统未初始化 */
#define HB_ERR_VOT_SYS_NOTREADY 0xA404
/* 设备ID 超出合法范围 */
#define HB_ERR_VOT_INVALID_DEVID 0xA405
/* 通道ID 超出合法范围 */
#define HB_ERR_VOT_INVALID_CHNID 0xA406
/* 参数超出合法范围 */
#define HB_ERR_VOT_ILLEGAL_PARAM 0xA407
/* 不支持的操作 */
#define HB_ERR_VOT_NOT_SUPPORT 0xA408
/* 操作不允许 */
#define HB_ERR_VOT_NOT_PERMIT 0xA409
/* WBC 号超出范围 */
#define HB_ERR_VOT_INVALID_WBCID 0xA40A
/* 视频层号超出范围 */
#define HB_ERR_VOT_INVALID_LAYERID 0xA40B
#define HB_ERR_VOT_INVALID_VIDEO_CHNID 0xA40C
/* 绑定VPS GROUP号超出范围 */
#define HB_ERR_VOT_INVALID_BIND_VPSGROUPID 0xA40D
/* 绑定VPS CHN号超出范围 */
#define HB_ERR_VOT_INVALID_BIND_VPSCHNID 0xA40E
#define HB_ERR_VOT_INVALID_FRAME_RATE 0xA40F

/* Device relative error code */
/* 设备未配置 */
#define HB_ERR_VOT_DEV_NOT_CONFIG 0xA410
/* 设备未使能 */
#define HB_ERR_VOT_DEV_NOT_ENABLE 0xA411
/* 设备已使能 */
#define HB_ERR_VOT_DEV_HAS_ENABLED 0xA412
/* 设备已被绑定 */
#define HB_ERR_VOT_DEV_HAS_BINDED 0xA413
/* 设备未被绑定 */
#define HB_ERR_VOT_DEV_NOT_BINDED 0xA414

#define HB_ERR_VOT_LAYER_NOT_ENABLE 0xA415

/* Video layer relative error code */
/* 视频层未使能 */
#define HB_ERR_VOT_VIDEO_NOT_ENABLE 0xA420
/* 视频层未禁止 */
#define HB_ERR_VOT_VIDEO_NOT_DISABLE 0xA421
/* 视频层未配置 */
#define HB_ERR_VOT_VIDEO_NOT_CONFIG 0xA422
/* 视频层已绑定 */
#define HB_ERR_VOT_VIDEO_HAS_BINDED 0xA423
/* 视频层未绑定 */
#define HB_ERR_VOT_VIDEO_NOT_BINDED 0xA424

/* WBC Relative error code */
/* 回写设备未禁用 */
#define HB_ERR_VOT_WBC_NOT_DISABLE 0xA430
/* 回写设备未配置 */
#define HB_ERR_VOT_WBC_NOT_CONFIG 0xA431
/* 回写设备已经配置 */
#define HB_ERR_VOT_WBC_HAS_CONFIG 0xA432
/* 回写设备未绑定 */
#define HB_ERR_VOT_WBC_NOT_BIND 0xA433
/* 回写设备已经绑定 */
#define HB_ERR_VOT_WBC_HAS_BIND 0xA434
/* */
#define HB_ERR_VOT_INVALID_WBID 0xA435
/* */
#define HB_ERR_VOT_WB_NOT_ENABLE 0xA436
#define HB_ERR_VOT_WB_GET_TIMEOUT 0xA437

/* Channel Relative error code */
/* 通道未禁止 */
#define HB_ERR_VOT_CHN_NOT_DISABLE 0xA440
/* 通道未使能 */
#define HB_ERR_VOT_CHN_NOT_ENABLE 0xA441
/* 通道未配置 */
#define HB_ERR_VOT_CHN_NOT_CONFIG 0xA442
/* 通道未分配资源 */
#define HB_ERR_VOT_CHN_NOT_ALLOC 0xA443
/* VO 通道区域重叠 */
#define HB_ERR_VOT_CHN_AREA_OVERLAP 0xA444

/* Cascade Relatvie error code */
/* 无效样式 */
#define HB_ERR_VOT_INVALID_PATTERN 0xA450
/* 无效级联位置 */
#define HB_ERR_VOT_INVALID_POSITION 0xA451

/* MISCellaneous error code */
/* 等待超时 */
#define HB_ERR_VOT_WAIT_TIMEOUT 0xA460
/* 无效视频帧 */
#define HB_ERR_VOT_INVALID_VFRAME 0xA461
/* 无效矩形参数 */
#define HB_ERR_VOT_INVALID_RECT_PARA 0xA462
/* BEGIN 已设置 */
#define HB_ERR_VOT_SETBEGIN_ALREADY 0xA463
/* BEGIN 没有设置 */
#define HB_ERR_VOT_SETBEGIN_NOTYET 0xA464
/* END已设置 */
#define HB_ERR_VOT_SETEND_ALREADY 0xA465
/* END没有设置 */
#define HB_ERR_VOT_SETEND_NOTYET 0xA466

/* Graphics layer relative error code */
/* 图形层未关闭 */
#define HB_ERR_VOT_GFX_NOT_DISABLE 0xA470
/* 图形层未绑定 */
#define HB_ERR_VOT_GFX_NOT_BIND 0xA471
/* 图形层未解绑定 */
#define HB_ERR_VOT_GFX_NOT_UNBIND 0xA472
/* 图形层ID 超出范围 */
#define HB_ERR_VOT_GFX_INVALID_ID 0xA473

/* Buffer manager relative error code */
#define HB_ERR_VOT_BUF_MANAGER_ILLEGAL_OPERATION 0xA480

#define VOT_MAX_DEV_NUM 1   /* max dev num */
#define VOT_MAX_LAYER_NUM 1 /* max layer num */
#define VOT_MAX_CHN_NUM 4   /* max chn num */
#define VOT_MAX_VIDEO_CHN_NUM 2
#define VOT_MAX_WBC_NUM 1   /* max wbc num */
#define VOT_MAX_PRIORITY 4
#define VOT_MAX_BIND_VPS_GROUP_NUM 4
#define VOT_MAX_BIND_VPS_CHN_NUM 39

typedef uint32_t VOT_WB;

// typedef enum {
// 	HB_FALSE = 0,
// 	HB_TRUE = 1,
// } HB_BOOL;

typedef struct HB_POINT_S {
	int s32X;
	int s32Y;
} POINT_S;

typedef struct HB_SIZE_S {
	uint32_t u32Width;
	uint32_t u32Height;
} SIZE_S;

// typedef struct HB_RECT_S {
//   int s32X;
//   int s32Y;
//   uint32_t u32Width;
//   uint32_t u32Height;
// } RECT_S;

// typedef uint32_t PIXEL_FORMAT_E;

typedef enum HB_PIXEL_FORMAT_YUV_E {
	PIXEL_FORMAT_YUV422_UYVY = 0,
	PIXEL_FORMAT_YUV422_VYUY = 1,
	PIXEL_FORMAT_YUV422_YVYU = 2,
	PIXEL_FORMAT_YUV422_YUYV = 3,
	PIXEL_FORMAT_YUV422SP_UV = 4,
	PIXEL_FORMAT_YUV422SP_VU = 5,
	PIXEL_FORMAT_YUV420SP_UV = 6,
	PIXEL_FORMAT_YUV420SP_VU = 7,
	PIXEL_FORMAT_YUV422P_UV = 8,
	PIXEL_FORMAT_YUV422P_VU = 9,
	PIXEL_FORMAT_YUV420P_UV = 10,
	PIXEL_FORMAT_YUV420P_VU = 11,
	PIXEL_FORMAT_YUV_BUTT = 12
} PIXEL_FORMAT_YUV_E;

typedef enum HB_PIXEL_FORMAT_RGB_E {
	PIXEL_FORMAT_8BPP = 0,
	PIXEL_FORMAT_RGB565 = 1,
	PIXEL_FORMAT_RGB888 = 2,
	PIXEL_FORMAT_RGB888P = 3,
	PIXEL_FORMAT_ARGB8888 = 4,
	PIXEL_FORMAT_RGBA8888 = 5,
	PIXEL_FORMAT_RGB_BUTT = 6
} PIXEL_FORMAT_RGB_E;

typedef enum HB_VOT_INTF_SYNC_E {
	VOT_OUTPUT_1920x1080,
	VOT_OUTPUT_800x480,
	VOT_OUTPUT_720x1280,
	VOT_OUTPUT_1080x1920,

	VO_OUTPUT_USER, /* User timing. */

	VO_OUTPUT_BUTT
} VOT_INTF_SYNC_E;

typedef struct HB_VOT_SYNC_INFO_S {
	uint32_t hbp;
	uint32_t hfp;
	uint32_t hs;
	uint32_t vbp;
	uint32_t vfp;
	uint32_t vs;
	uint32_t vfp_cnt;
} VOT_SYNC_INFO_S;

typedef enum HB_VOT_OUTPUT_MODE_E {
	HB_VOT_OUTPUT_MIPI,
	HB_VOT_OUTPUT_BT1120,
	HB_VOT_OUTPUT_RGB888,

	HB_VOT_OUTPUT_MODE_BUTT
} VOT_OUTPUT_MODE_E;

typedef struct HB_VOT_CROP_INFO_S {
	uint32_t u32Width;
	uint32_t u32Height;
} VOT_CROP_INFO_S;

typedef struct HB_VOT_PUB_ATTR_S {
	uint32_t u32BgColor;            /* 设备背景色 RGB表示 */
	VOT_OUTPUT_MODE_E enOutputMode; /* Vo 接口类型 */
	VOT_INTF_SYNC_E enIntfSync;     /* Vo接口时序类型 */
	VOT_SYNC_INFO_S stSyncInfo;     /* Vo接口时序信息 */
} VOT_PUB_ATTR_S;

typedef struct HB_VOT_VIDEO_LAYER_ATTR_S {
	SIZE_S stImageSize; /* 视频层画布大小 */
	uint32_t  big_endian;
	uint32_t	display_addr_type;
	uint32_t	display_cam_no;
	uint32_t  display_addr_type_layer1;
	uint32_t  display_cam_no_layer1;

	uint32_t	dithering_flag;
	uint32_t	dithering_en;
	uint32_t	gamma_en;
	uint32_t	hue_en;
	uint32_t	sat_en;
	uint32_t	con_en;
	uint32_t	bright_en;
	uint32_t	theta_sign;
	uint32_t	contrast;
	uint32_t 	gamma;

	uint32_t	theta_abs;  // ppcon2
	uint32_t	saturation;
	uint32_t	off_contrast;
	uint32_t	off_bright;

	uint32_t	panel_type;
	uint32_t	rotate;
	uint32_t	user_control_disp;
	uint32_t	user_control_disp_layer1;
} VOT_VIDEO_LAYER_ATTR_S;

typedef struct HB_VOT_CSC_S {
	uint32_t u32Luma;
	uint32_t u32Contrast;
	uint32_t u32Hue;
	uint32_t u32Satuature;
} VOT_CSC_S;

typedef struct HB_VOT_UPSCALE_ATTR_S {
	uint32_t src_width;
	uint32_t src_height;
	uint32_t tgt_width;
	uint32_t tgt_height;
	uint32_t pos_x;
	uint32_t pos_y;
	uint32_t upscale_en;
} VOT_UPSCALE_ATTR_S;

typedef struct HB_VOT_CHN_ATTR_S {
	uint32_t u32Priority;
	uint32_t u32SrcWidth;
	uint32_t u32SrcHeight;
	int s32X;
	int s32Y;
	uint32_t u32DstWidth;
	uint32_t u32DstHeight;
} VOT_CHN_ATTR_S;

typedef struct HB_VOT_CHN_ATTR_EX_S {
	uint32_t format;     // yuv格式
	uint32_t alpha_en;   //
	uint32_t alpha_sel;  // alpha公式选择
	uint32_t alpha;
	uint32_t keycolor;  // 叠加背景色
						/*ov_mode
						00 transparent 透明
						01 and 与
						10 or 或
						11 inv
						*/
	uint32_t ov_mode;   // 叠加模式
} VOT_CHN_ATTR_EX_S;

// typedef enum HB_VOT_ROTATION_E {
//   ROTATION_0 = 0,
//   ROTATION_90 = 1,
//   ROTATION_180 = 2,
//   ROTATION_270 = 3,
//   ROTATION_BUTT
// } VOT_ROTATION_E;

typedef struct HB_VOT_WBC_ATTR_S {
	int wb_src;
	int wb_format;
} VOT_WB_ATTR_S;

typedef struct HB_VOT_FRAME_INFO_S {
	void *addr;
	unsigned int size;
} VOT_FRAME_INFO_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HB_COMM_VO_H__ */
