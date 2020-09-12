#ifndef __HB_ISP_H__
#define __HB_ISP_H__

#include <stdint.h>
#include "hb_isp_algo.h"

// ------------------------------------------------------------------------------ //
//		VALUE LIST
// ------------------------------------------------------------------------------ //
#define NORMAL                                            0x0000003E
#define BLACK_AND_WHITE                                   0x0000003F
#define NEGATIVE                                          0x00000040
#define SEPIA                                             0x00000041
#define VIVID                                             0x00000042

/*****************************************************************************************
 *					Control API 				 	 *
 *****************************************************************************************/
typedef enum HB_ISP_OP_TYPE_E {
	OP_TYPE_AUTO = 0,
	OP_TYPE_MANUAL,
} ISP_OP_TYPE_E;

typedef enum HB_ISP_FW_STATE_E {
	ISP_FW_STATE_RUN = 0,
	ISP_FW_STATE_FREEZE,
} ISP_FW_STATE_E;

typedef union HB_ISP_MODULE_CTRL_U {
	uint64_t u64Value;
	struct {
		uint64_t bitBypassVideoTestGen			: 1 ;	/* 0x18EAC */
		uint64_t bitBypassInputFormatter		: 1 ;
		uint64_t bitBypassDecompander			: 1 ;
		uint64_t bitBypassSensorOffsetWDR		: 1 ;
		uint64_t bitBypassGainWDR			: 1 ;
		uint64_t bitBypassFrameStitch			: 1 ;
		uint64_t bitBypassDigitalGain			: 1 ;	/* 0x18EB0 */
		uint64_t bitBypassFrontendSensorOffset		: 1 ;
		uint64_t bitBypassFeSqrt			: 1 ;
		uint64_t bitBypassRAWFrontend			: 1 ;
		uint64_t bitBypassDefectPixel			: 1 ;
		uint64_t bitBypassSinter			: 1 ;	/* 0x18EB8 */
		uint64_t bitBypassTemper			: 1 ;
		uint64_t bitBypassCaCorrection			: 1 ;
		uint64_t bitBypassSquareBackend			: 1 ;	/* 0x18EBC */
		uint64_t bitBypassSensorOffsetPreShading	: 1 ;
		uint64_t bitBypassRadialShading			: 1 ;
		uint64_t bitBypassMeshShading			: 1 ;
		uint64_t bitBypassWhiteBalance			: 1 ;
		uint64_t bitBypassIridixGain			: 1 ;
		uint64_t bitBypassIridix			: 1 ;
		uint64_t bitBypassMirror			: 1 ;	/* 0x18EC0 */
		uint64_t bitBypassDemosaicRGB			: 1 ;
		uint64_t bitBypassPfCorrection			: 1 ;
		uint64_t bitBypassCCM				: 1 ;
		uint64_t bitBypassCNR				: 1 ;
		uint64_t bitBypass3Dlut				: 1 ;
		uint64_t bitBypassNonequGamma			: 1 ;
		uint64_t bitBypassFrCrop			: 1 ;	/* 0x18EC4 */
		uint64_t bitBypassFrGammaRGB			: 1 ;
		uint64_t bitBypassFrSharpen			: 1 ;
		uint64_t bitBypassFrCsConv			: 1 ;
		uint64_t bitBypassRAW				: 1 ;	/* [32] 0x18ECC */
		uint64_t bitRsv					: 31 ;	/* [33:63]*/
	};
} ISP_MODULE_CTRL_U;

extern int HB_ISP_GetSetInit(void);
extern int HB_ISP_GetSetExit(void);

extern int HB_ISP_SetFWState(uint8_t pipeId, const ISP_FW_STATE_E enState);
extern int HB_ISP_GetFWState(uint8_t pipeId, ISP_FW_STATE_E *penState);

extern int HB_ISP_SetModuleControl(uint8_t pipeId, const ISP_MODULE_CTRL_U *punModCtrl);
extern int HB_ISP_GetModuleControl(uint8_t pipeId, ISP_MODULE_CTRL_U *punModCtrl);

extern int HB_ISP_SetRegister(uint8_t pipeId, uint32_t u32Addr, uint32_t u32Value);
extern int HB_ISP_GetRegister(uint8_t pipeId, uint32_t u32Addr, uint32_t *pu32Value);

extern int HB_ISP_SwitchScence(uint8_t pipeId, const char *cname);

/*****************************************************************************************
 *				  3ALib Register API			 	 	 *
 *****************************************************************************************/
/* AE */
typedef enum ae_state {
    AE_STATE_INACTIVE,
    AE_STATE_SEARCHING,
    AE_STATE_CONVERGED,
} ae_state_t;

typedef struct _ae_stats_data_ {
	uint32_t *fullhist;
	uint32_t fullhist_size;
	uint32_t fullhist_sum;
	uint16_t *zone_hist;
	uint32_t zone_hist_size;
} ae_stats_data_t;

typedef struct _ae_input_data_ {
	void *custom_input;
	void *acamera_input;
} ae_input_data_t;

typedef struct _ae_output_data_ {
	void *custom_output;
	void *acamera_output;
} ae_output_data_t;

typedef struct _ae_acamera_output_ {
	int32_t exposure_log2;
	uint32_t exposure_ratio;
	uint8_t ae_converged;
	uint16_t sensor_ctrl_enable;
	ae_out_info_t ae_out_info;
	ae_1024bin_weight_t ae_1024bin_weight;
} ae_acamera_output_t; 

typedef struct HB_ISP_AE_FUNC_S {
	void *(*init_func)(uint32_t ctx_id);
	int32_t (*proc_func)(void *ae_ctx, ae_stats_data_t *stats, ae_input_data_t *input, ae_output_data_t *output);
	int32_t (*deinit_func)(void *ae_ctx);
} ISP_AE_FUNC_S;

/* AWB */
typedef enum awb_state {
    AWB_STATE_INACTIVE,
    AWB_STATE_SEARCHING,
    AWB_STATE_CONVERGED,
} awb_state_t;

typedef struct _awb_zone_t {
	uint16_t rg;
	uint16_t bg;
	uint32_t sum;
} awb_zone_t;

typedef struct _awb_stats_data_ {
	awb_zone_t *awb_zones;
	uint32_t zones_size;
} awb_stats_data_t;

typedef struct _awb_input_data_ {
	void *custom_input;
	void *acamera_input;
} awb_input_data_t;

typedef struct _awb_output_data_ {
	void *custom_output;
	void *acamera_output;
} awb_output_data_t;

typedef struct _awb_acamera_output_ {
	uint16_t rg_coef;
	uint16_t bg_coef;
	int32_t temperature_detected;
	uint8_t p_high;
	uint8_t light_source_candidate;
	int32_t awb_warming[3];
	uint8_t awb_converged;
} awb_acamera_output_t;

typedef struct HB_ISP_AWB_FUNC_S {
	void *(*init_func)(uint32_t ctx_id);
	int32_t (*proc_func)(void *awb_ctx, awb_stats_data_t *stats, awb_input_data_t *input, awb_output_data_t *output);
	int32_t (*deinit_func)(void *awb_ctx);
} ISP_AWB_FUNC_S;

/* AF */
typedef enum af_state {
	AF_STATE_INACTIVE,
	AF_STATE_SCAN,
	AF_STATE_FOCUSED,
	AF_STATE_UNFOCUSED
} af_state_t;

typedef struct _af_stats_data_ {
	uint32_t *zones_stats;
	uint32_t zones_size;
} af_stats_data_t;

typedef struct _af_input_data_ {
	void *custom_input;
	void *acamera_input;
} af_input_data_t;

typedef struct _af_output_data_ {
	void *custom_output;
	void *acamera_output;
} af_output_data_t;

typedef struct _af_acamera_output_ {
	uint16_t af_lens_pos;
	int32_t af_sharp_val;
	af_state_t state;
} af_acamera_output_t;

typedef struct HB_ISP_AF_FUNC_S {
	void *(*init_func)(uint32_t ctx_id);
	int32_t (*proc_func)(void *af_ctx, af_stats_data_t *stats, af_input_data_t *input, af_output_data_t *output);
	int32_t (*deinit_func)(void *af_ctx);
} ISP_AF_FUNC_S;

extern int HB_ISP_AELibRegCallback(uint8_t pipeId, char *name, ISP_AE_FUNC_S *pstAeFunc);
extern int HB_ISP_AWBLibRegCallback(uint8_t pipeId, char *name, ISP_AWB_FUNC_S *pstAWBFunc);
extern int HB_ISP_AFLibRegCallback(uint8_t pipeId, char *name, ISP_AF_FUNC_S *pstAFFunc);

extern int HB_ISP_AELibUnRegCallback(uint8_t pipeId);
extern int HB_ISP_AWBLibUnRegCallback(uint8_t pipeId);
extern int HB_ISP_AFLibUnRegCallback(uint8_t pipeId);

/*****************************************************************************************
 *				  Module Tune API 				 	 *
 *****************************************************************************************/
#define ISP_AUTO_ISO_STRENGTH_NUM	16

/* AE */ //cmd
typedef struct HB_ISP_AE_ATTR_S {
	uint32_t u32Exposure;
	uint32_t u32ExposureRatio;
	uint32_t u32IntegrationTime;
//	uint32_t u32LongIntegrationTime;
//	uint32_t u32ShortIntegrationTime;
	uint32_t u32SensorAnalogGain;
	uint32_t u32SensorDigitalGain;
	uint32_t u32IspDigitalGain;
	uint32_t u32MaxExposureRatio;
	uint32_t u32MaxIntegrationTime;
	uint32_t u32MaxSensorAnalogGain;
	uint32_t u32MaxSensorDigitalGain;
	uint32_t u32MaxIspDigitalGain;
	ISP_OP_TYPE_E enOpType;
} ISP_AE_ATTR_S;

/*AF*/ // cmd
typedef struct HB_ISP_AF_ATTR_S {
	uint32_t u32ZoomPos;
	ISP_OP_TYPE_E enOpType;
} ISP_AF_ATTR_S;

/* AWB */ //cmd
typedef struct HB_ISP_AWB_ATTR_S {
	uint32_t u16RGain;
	uint32_t u16BGain;
	ISP_OP_TYPE_E enOpType;
} ISP_AWB_ATTR_S;

/* Black Level */
typedef struct HB_ISP_BLACK_LEVEL_ATTR_S {
	//reserved struct
} ISP_BLACK_LEVEL_ATTR_S;

/* Demosaic */ //reg + lut
typedef struct HB_ISP_DEMOSAIC_ATTR_S {
	uint32_t u32FcSlope;
	uint32_t u32FcAliasSlope;
	uint32_t u32FcAliasThresh;
	uint16_t u16NpOffset[ISP_AUTO_ISO_STRENGTH_NUM][2];
} ISP_DEMOSAIC_ATTR_S;

/* Sharpen */ //lut
typedef struct HB_ISP_SHARPEN_ATTR_S {
	uint16_t u16SharpFR[ISP_AUTO_ISO_STRENGTH_NUM][2];
	uint16_t u16SharpAltD[ISP_AUTO_ISO_STRENGTH_NUM][2];
	uint16_t u16SharpAltDU[ISP_AUTO_ISO_STRENGTH_NUM][2];
	uint16_t u16SharpAltUD[ISP_AUTO_ISO_STRENGTH_NUM][2];
} ISP_SHARPEN_ATTR_S;

/* Gamma */ //lut
typedef struct HB_ISP_GAMMA_ATTR_S {
	uint16_t au16Gamma[129];
} ISP_GAMMA_ATTR_S;

/* Iridix */ //lut
typedef struct HB_ISP_IRIDIX_ATTR_S {
	uint8_t u8AvgCoef;
	uint32_t au32EvLimNoStr[2];
	uint32_t u32EvLimFullStr;
	uint32_t au32StrengthDkEnhControl[15];
} ISP_IRIDIX_ATTR_S;

/* CNR */ //lut
typedef struct HB_ISP_CNR_ATTR_S {
	uint16_t u16UvDelta12Slope[ISP_AUTO_ISO_STRENGTH_NUM][2];
} ISP_CNR_ATTR_S;

/* Sinter */ //lut
typedef struct HB_ISP_SINTER_ATTR_S {
	uint16_t aau16Strength[ISP_AUTO_ISO_STRENGTH_NUM][2];
	uint16_t aau16Strength1[ISP_AUTO_ISO_STRENGTH_NUM][2];
	uint16_t aau16Thresh1[ISP_AUTO_ISO_STRENGTH_NUM][2];
	uint16_t aau16Thresh4[ISP_AUTO_ISO_STRENGTH_NUM][2];
} ISP_SINTER_ATTR_S;

/* Temper */ //lut + reg
typedef struct HB_ISP_TEMPER_ATTR_S {
	uint16_t aau16Strength[ISP_AUTO_ISO_STRENGTH_NUM][2];
	uint32_t u32RecursionLimit;
} ISP_TEMPER_ATTR_S;

/* Scene Mode */ //cmd
typedef struct HB_ISP_SCENE_MODES_ATTR_S {
	uint32_t u32ColorMode;
	uint32_t u32BrightnessStrength;
	uint32_t u32ContrastStrength;
	uint32_t u32SaturationStrength;
	uint32_t u32SharpeningStrength;
} ISP_SCENE_MODES_ATTR_S;

/* AE */
extern int HB_ISP_SetAeAttr(uint8_t pipeId, const ISP_AE_ATTR_S *pstAeAttr);
extern int HB_ISP_GetAeAttr(uint8_t pipeId, ISP_AE_ATTR_S *pstAeAttr);

/* AWB */
extern int HB_ISP_SetAwbAttr(uint8_t pipeId, const ISP_AWB_ATTR_S *pstAwbAttr);
extern int HB_ISP_GetAwbAttr(uint8_t pipeId, ISP_AWB_ATTR_S *pstAwbAttr);

/* Black Level */
extern int HB_ISP_SetBlackLevelAttr(uint8_t pipeId, const ISP_BLACK_LEVEL_ATTR_S *pstBlackLevelAttr);
extern int HB_ISP_GetBlackLevelAttr(uint8_t pipeId, ISP_BLACK_LEVEL_ATTR_S *pstBlackLevelAttr);

/* Demosaic */
extern int HB_ISP_SetDemosaicAttr(uint8_t pipeId, const ISP_DEMOSAIC_ATTR_S *pstDemosaicAttr);
extern int HB_ISP_GetDemosaicAttr(uint8_t pipeId, ISP_DEMOSAIC_ATTR_S *pstDemosaicAttr);

/* Sharpen */
extern int HB_ISP_SetSharpenAttr(uint8_t pipeId, const ISP_SHARPEN_ATTR_S *pstSharpenAttr);
extern int HB_ISP_GetSharpenAttr(uint8_t pipeId, ISP_SHARPEN_ATTR_S *pstSharpenAttr);

/* Gamma */
extern int HB_ISP_SetGammaAttr(uint8_t pipeId, const ISP_GAMMA_ATTR_S *pstGammaAttr);
extern int HB_ISP_GetGammaAttr(uint8_t pipeId, ISP_GAMMA_ATTR_S *pstGammaAttr);

/* Iridix */
extern int HB_ISP_SetIridixAttr(uint8_t pipeId, const ISP_IRIDIX_ATTR_S *pstIridixAttr);
extern int HB_ISP_GetIridixAttr(uint8_t pipeId, ISP_IRIDIX_ATTR_S *pstIridixAttr);

/* CNR */
extern int HB_ISP_SetCnrAttr(uint8_t pipeId, const ISP_CNR_ATTR_S *pstCnrAttr);
extern int HB_ISP_GetCnrAttr(uint8_t pipeId, ISP_CNR_ATTR_S *pstCnrAttr);

/* Sinter */
extern int HB_ISP_SetSinterAttr(uint8_t pipeId, const ISP_SINTER_ATTR_S *pstSinterAttr);
extern int HB_ISP_GetSinterAttr(uint8_t pipeId, ISP_SINTER_ATTR_S *pstSinterAttr);

/* Temper */
extern int HB_ISP_SetTemperAttr(uint8_t pipeId, const ISP_TEMPER_ATTR_S *pstTemperAttr);
extern int HB_ISP_GetTemperAttr(uint8_t pipeId, ISP_TEMPER_ATTR_S *pstTemperAttr);

/* Scene Mode */
extern int HB_ISP_SetSceneModesAttr(uint8_t pipeId, const ISP_SCENE_MODES_ATTR_S *pstSceneModesAttr);
extern int HB_ISP_GetSceneModesAttr(uint8_t pipeId, ISP_SCENE_MODES_ATTR_S *pstSceneModesAttr);

/* Statistics */
#define HB_ISP_MAX_AWB_ZONES (33 * 33)
#define HB_ISP_AF_ZONES_COUNT_MAX (33 * 33)
#define HB_ISP_FULL_HISTOGRAM_SIZE 1024

typedef struct HB_ISP_STATISTICS_AWB_ZONE_ATTR_S {
	uint16_t u16Rg;
	uint16_t u16Bg;
	uint32_t u32Sum;
} ISP_STATISTICS_AWB_ZONE_ATTR_S;

extern int HB_ISP_GetAeFullHist(uint8_t pipeId, uint32_t *pu32AeFullHist);
extern int HB_ISP_GetAwbZone(uint8_t pipeId, ISP_STATISTICS_AWB_ZONE_ATTR_S *pstAwbZonesAttr);

#endif
