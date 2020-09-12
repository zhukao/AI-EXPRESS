/*****************************************************************************
 * Copyright (c) 2019 Horizon Robotics.
 * Description: HAPI 视频输入模块头文件
 * Version:v0.1
 * Date: 2019-12-12 14:27:20
 * LastEditTime: 2020-01-09 10:55:54
 * History:
 *****************************************************************************/
#ifndef __HB_VIN_H__
#define __HB_VIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define RET_OK 0
#define RET_ERROR 1

enum HB_VIN_ERROR_CODE {
	HB_VIN_SIF_INIT_FAIL = 1,
	HB_VIN_PARAM_INIT_FAIL,
	HB_VIN_DEV_START_FAIL,
	HB_VIN_PIPE_START_FAIL,
	HB_VIN_CHN_UNEXIST,
	HB_VIN_INVALID_PARAM,
	HB_VIN_ISP_INIT_FAIL,
	HB_VIN_ISP_MODULE_INIT_FAIL,
	HB_VIN_CHANNEL_INIT_FAIL,
	HB_VIN_MALLOC_BUFMGR_FAIL,
	HB_VIN_DWE_INIT_FAIL,
	HB_VIN_BIND_FAIL,
	HB_VIN_SET_DEV_ATTREX_FAIL,
	HB_VIN_LENS_INIT_FAIL,
	HB_VIN_SEND_PIPERAW_FAIL,
	HB_VIN_NULL_POINT,
	HB_VIN_GET_CHNFRAME_FAIL,
	HB_VIN_GET_DEVFRAME_FAIL,
	HB_VIN_MD_ENABLE_FAIL,
	HB_VIN_MD_DISABLE_FAIL,
};

typedef enum HB_VIN_DEV_INTF_MODE_E {
    VIN_MODE_MIPI,                   /* MIPI RAW mode */
    VIN_MODE_DVP,                    /* DVP mode */
    VIN_MODE_BUTT
} VIN_DEV_INTF_MODE_E;

typedef struct HB_VIN_DEV_SIZE_S {
	uint32_t  format;
	uint32_t  width;
	uint32_t  height;
	uint32_t  pix_length;
} VIN_DEV_SIZE_S;

typedef struct HB_VIN_MIPI_ATTR_S {
	uint32_t  enable;
	uint32_t  ipi_channels;
	uint32_t  ipi_mode;
	// uint32_t  vc_index;
	// uint32_t  mipi_rx_index;
	uint32_t  enable_mux_out;
	// uint32_t  mux_out_index;
	uint32_t  enable_frame_id;
	uint32_t  enable_bypass;
	uint32_t  enable_line_shift;
	uint32_t  enable_id_decoder;
	uint32_t  set_init_frame_id;
	uint32_t  set_line_shift_count;
	uint32_t  set_bypass_channels;
	uint32_t  enable_pattern;
} VIN_MIPI_ATTR_S;

typedef struct HB_VIN_DVP_ATTR_S {
} VIN_DVP_ATTR_S;

typedef struct HB_VIN_DEV_OUTPUT_DDR_S {
	// uint32_t mux_out_enable_index;
	uint32_t stride;
	uint32_t buffer_num;
	uint32_t raw_dump_en;
} VIN_DEV_OUTPUT_DDR_S;

typedef struct HB_VIN_DEV_OUTPUT_ISP_S {
	uint32_t dol_exp_num;
	uint32_t enable_dgain;
	uint32_t set_dgain_short;
	uint32_t set_dgain_medium;
	uint32_t set_dgain_long;
	uint32_t short_maxexp_lines;
	uint32_t medium_maxexp_lines;
	uint32_t vc_short_seq;  // frame sequence mark in dol mode
	uint32_t vc_medium_seq;
	uint32_t vc_long_seq;
} VIN_DEV_OUTPUT_ISP_S;

typedef struct HB_VIN_DEV_INPUT_DDR_ATTR_S {
	uint32_t stride;
	uint32_t buf_num;
	uint32_t raw_feedback_en;
	VIN_DEV_SIZE_S data;
} VIN_DEV_INPUT_DDR_ATTR_S;

typedef struct HB_VIN_DEV_ATTR_EX_S {
	uint32_t path_sel;
	uint32_t roi_top;
	uint32_t roi_left;
	uint32_t roi_width;
	uint32_t roi_height;
	uint32_t grid_step;
	uint32_t grid_tolerance;
	uint32_t threshold;
	uint32_t weight_decay;
	uint32_t precision;
}VIN_DEV_ATTR_EX_S;

typedef struct HB_VIN_DEV_ATTR_S {
	// VIN_DEV_INTF_MODE_E   enIntfMode;
	VIN_DEV_SIZE_S        stSize;
	union
	{
		VIN_MIPI_ATTR_S  mipiAttr;
		VIN_DVP_ATTR_S   dvpAttr;
	};
	VIN_DEV_INPUT_DDR_ATTR_S DdrIspAttr;
	VIN_DEV_OUTPUT_DDR_S outDdrAttr;
	VIN_DEV_OUTPUT_ISP_S outIspAttr;
}VIN_DEV_ATTR_S;

typedef struct HB_VIN_PIPE_SIZE_S {
	uint32_t  format;
	uint32_t  width;
	uint32_t  height;
} VIN_PIPE_SIZE_S;

typedef enum HB_VIN_PIPE_SENSOR_MODE_E {
	SENSOR_NORMAL_MODE = 1,
	SENSOR_DOL2_MODE,
	SENSOR_DOL3_MODE,
	SENSOR_DOL4_MODE,
	SENSOR_PWL_MODE,
	SENSOR_INVAILD_MODE
} VIN_PIPE_SENSOR_MODE_E;

typedef enum HB_VIN_PIPE_CFA_PATTERN_E {
	PIPE_BAYER_RGGB = 0,
	PIPE_BAYER_GRBG,
	PIPE_BAYER_GBRG,
	PIPE_BAYER_BGGR,
	PIPE_MONOCHROME,
} VIN_PIPE_CFA_PATTERN_E;

typedef struct HB_VIN_PIPE_CALIB_S {
	uint32_t mode;
	char *lname;
} VIN_PIPE_CALIB_S;

typedef struct HB_VIN_PIPE_ATTR_S {
	uint32_t  ddrOutBufNum;
	VIN_PIPE_SENSOR_MODE_E snsMode;
	VIN_PIPE_SIZE_S stSize;
	VIN_PIPE_CFA_PATTERN_E cfaPattern;
	uint32_t   temperMode;
	uint32_t   ispBypassEn;
	uint32_t   ispAlgoState;
	uint32_t   bitwidth;
	uint32_t   startX;
	uint32_t   startY;
	VIN_PIPE_CALIB_S calib;
} VIN_PIPE_ATTR_S;

typedef struct HB_VIN_LDC_PATH_SEL_S {
	uint32_t rg_y_only:1;
	uint32_t rg_uv_mode:1;
	uint32_t rg_uv_interpo:1;
	uint32_t reserved1:5;
	uint32_t rg_h_blank_cyc:8;
	uint32_t reserved0:16;
} VIN_LDC_PATH_SEL_S;

typedef struct HB_VIN_LDC_PICSIZE_S {
	uint16_t pic_w;
	uint16_t pic_h;
} VIN_LDC_PICSIZE_S;

typedef struct HB_VIN_LDC_ALGOPARAM_S {
	uint16_t rg_algo_param_b;
	uint16_t rg_algo_param_a;
} VIN_LDC_ALGOPARAM_S;

typedef struct HB_VIN_LDC_OFF_SHIFT_S {
	uint32_t rg_center_xoff:8;
	uint32_t rg_center_yoff:8;
	uint32_t reserved0:16;
} VIN_LDC_OFF_SHIFT_S;

typedef struct HB_VIN_LDC_WOI_S {
	uint32_t rg_start:12;
	uint32_t reserved1:4;
	uint32_t rg_length:12;
	uint32_t reserved0:4;
}VIN_LDC_WOI_S;

typedef struct HB_VIN_LDC_ATTR_S {
	uint32_t         ldcEnable;
	VIN_LDC_PATH_SEL_S  ldcPath;
	uint32_t yStartAddr;
	uint32_t cStartAddr;
	VIN_LDC_PICSIZE_S  picSize;
	uint32_t lineBuf;
	VIN_LDC_ALGOPARAM_S xParam;
	VIN_LDC_ALGOPARAM_S yParam;
	VIN_LDC_OFF_SHIFT_S offShift;
	VIN_LDC_WOI_S   xWoi;
	VIN_LDC_WOI_S   yWoi;
} VIN_LDC_ATTR_S;

typedef struct HB_VIN_DIS_PICSIZE_S {
	uint16_t pic_w;
	uint16_t pic_h;
} VIN_DIS_PICSIZE_S;

typedef struct HB_VIN_DIS_PATH_SEL_S {
	uint32_t rg_dis_enable:1;
	uint32_t rg_dis_path_sel:1;
	uint32_t reserved0:30;
} VIN_DIS_PATH_SEL_S;

typedef struct HB_VIN_DIS_CROP_S {
	uint16_t rg_dis_start;
	uint16_t rg_dis_end;
} VIN_DIS_CROP_S;

typedef struct HB_VIN_DIS_ATTR_S {
	VIN_DIS_PICSIZE_S picSize;
	VIN_DIS_PATH_SEL_S disPath;
	uint32_t disHratio;
	uint32_t disVratio;
	VIN_DIS_CROP_S xCrop;
	VIN_DIS_CROP_S yCrop;
	uint32_t disBufNum;
} VIN_DIS_ATTR_S;

typedef struct HB_VIN_DIS_MV_INFO_S {
	int	gmvX;
	int	gmvY;
	int xUpdate;
	int yUpdate;
} VIN_DIS_MV_INFO_S;

typedef struct HB_VIN_DIS_CALLBACK_S {
    void (*VIN_DIS_DATA_CB) (uint32_t pipeId, uint32_t event,
             VIN_DIS_MV_INFO_S *disData, void *userData);
} VIN_DIS_CALLBACK_S;

typedef enum HB_VIN_LENS_MOTOR_TYPE_E {
	 VIN_LENS_PWM_TYPE = 0,
     VIN_LENS_PULSE_TYPE,
     VIN_LENS_I2C_TYPE,
     VIN_LENSSPI_TYPE,
     VIN_LENS_GPIO_TYPE
} VIN_LENS_MOTOR_TYPE_E;

typedef enum HB_VIN_LENS_FUNC_TYPE_E {
	VIN_LENS_AF_TYPE = 1,
	VIN_LENS_ZOOM_TYPE,
	VIN_LENS_INVALID,
} VIN_LENS_FUNC_TYPE_E;

typedef struct HB_VIN_LENS_CTRL_ATTR_S {
        uint16_t port;
        VIN_LENS_MOTOR_TYPE_E motorType;   // 0: pulses, 1 :pwm, 2: i2c 3: spi 4: gpio
        uint32_t maxStep;
        uint32_t initPos;
        uint32_t minPos;
        uint32_t maxPos;
        union {
                struct {
                        uint16_t pwmNum;
                        uint32_t pwmDuty;
                        uint32_t pwmPeriod;
                } pwmParam;
                struct {
                        uint16_t pulseForwardNum;
                        uint16_t pulseBackNum;
                        uint32_t pulseDuty;
                        uint32_t pulsePeriod;
                } pulseParam;
                struct {
                        uint16_t i2cNum;
                        uint32_t i2cAddr;
                } i2cParam;
                struct {
                        uint16_t gpioA1;  // A+
                        uint16_t gpioA2;  // A-
                        uint16_t gpioB1;  // B+
                        uint16_t gpioB2;  // B-
                } gpioParam;
        };
} VIN_LENS_CTRL_ATTR_S;

extern int HB_VIN_SetDevAttr(uint32_t devId,
			const VIN_DEV_ATTR_S *stVinDevAttr);
extern int HB_VIN_GetDevAttr(uint32_t devId,
			VIN_DEV_ATTR_S *stVinDevAttr);
extern int HB_VIN_SetDevMclk(uint32_t devId,
	uint32_t devMclk, uint32_t vpuMclk);
extern int HB_VIN_SetDevAttrEx(uint32_t devId, const VIN_DEV_ATTR_EX_S
*stVinDevAttrEx);
extern int HB_VIN_GetDevAttrEx(uint32_t devId, VIN_DEV_ATTR_EX_S
*stVinDevAttrEx);

extern int HB_VIN_EnableDevMd(uint32_t devId);
extern int HB_VIN_DisableDevMd(uint32_t devId);

extern int HB_VIN_EnableDev(uint32_t devId);
extern int HB_VIN_DisableDev(uint32_t devId);
extern int HB_VIN_DestroyDev(uint32_t devId);
extern int HB_VIN_GetDevFrame(uint32_t devId, uint32_t chnId,
		void *videoFrame, int32_t millSec);
extern int HB_VIN_ReleaseDevFrame(uint32_t devId, uint32_t chnId, void *buf);

extern int HB_VIN_SetDevVCNumber(uint32_t devId, uint32_t vcNumber);
extern int HB_VIN_GetDevVCNumber(uint32_t devId, uint32_t *vcNumber);
extern int HB_VIN_AddDevVCNumber(uint32_t devId, uint32_t vcNumber);

extern int HB_VIN_CreatePipe(uint32_t pipeId,
         const VIN_PIPE_ATTR_S *stVinPipeAttr);
extern int HB_VIN_SetPipeAttr(uint32_t pipeId,
         VIN_PIPE_ATTR_S *stVinPipeAttr);
extern int HB_VIN_GetPipeAttr(uint32_t pipeId,
         VIN_PIPE_ATTR_S *stVinPipeAttr);
extern int HB_VIN_StartPipe(uint32_t pipeId);
extern int HB_VIN_StopPipe(uint32_t pipeId);
extern int HB_VIN_SharePipeAE(uint32_t sharerPipeId, uint32_t userPipeId);
extern int HB_VIN_CancelSharePipeAE(uint32_t sharerPipeId);
extern int HB_VIN_DestroyPipe(uint32_t pipeId);
extern int HB_VIN_EnableChn(uint32_t pipeId, uint32_t chnId);
extern int HB_VIN_DisableChn(uint32_t pipeId, uint32_t chnId);
extern int HB_VIN_DestroyChn(uint32_t pipeId, uint32_t chnId);

extern int HB_VIN_GetChnFd(uint32_t pipeId, uint32_t chnId);
extern int HB_VIN_CloseFd(void);

extern int HB_VIN_SendPipeRaw(int pipeId, void *videoFrame, int32_t millSec);
extern int HB_VIN_GetChnFrame(uint32_t pipeId,
           uint32_t chnId, void *pstVideoFrame, int32_t millSec);
extern int HB_VIN_ReleaseChnFrame(uint32_t pipeId,
        uint32_t chnId, void *pstVideoFrame);

extern int HB_VIN_SetMipiBindDev(uint32_t devId, uint32_t mipiIdx);
extern int HB_VIN_GetMipiBindDev(uint32_t devId, uint32_t *mipiIdx);

extern int HB_VIN_SetDevBindPipe(uint32_t devId, uint32_t pipeId);
extern int HB_VIN_GetDevBindPipe(uint32_t devId, uint32_t *pipeId);

extern int HB_VIN_SetChnDISAttr(uint32_t pipeId, uint32_t chnId,
    const VIN_DIS_ATTR_S *stVinDisAttr);
extern int HB_VIN_GetChnDISAttr(uint32_t pipeId, uint32_t chnId,
      VIN_DIS_ATTR_S *stVinDisAttr);
extern int HB_VIN_SetChnLDCAttr(uint32_t pipeId, uint32_t chnId,
     const VIN_LDC_ATTR_S *stVinLdcAttr);
extern int HB_VIN_GetChnLDCAttr(uint32_t pipeId, uint32_t chnId,
        VIN_LDC_ATTR_S *stVinLdcAttr);
extern int HB_VIN_SetChnAttr(uint32_t pipeId, uint32_t chnId);
extern int HB_VIN_MotionDetect(uint32_t pipeId);

extern int HB_VIN_InitLens(uint32_t pipeId, VIN_LENS_FUNC_TYPE_E lensType,
	const VIN_LENS_CTRL_ATTR_S *lenCtlAttr);
extern int HB_VIN_DeinitLens(uint32_t pipeId);

extern int HB_VIN_RegisterDisCallback(uint32_t pipeId,
	VIN_DIS_CALLBACK_S *pstDISCallback);


#ifdef __cplusplus
}
#endif

#endif  // __HB_VIN_H__

