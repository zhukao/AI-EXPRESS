#ifndef __HB_ISP_ALGO_H__
#define __HB_ISP_ALGO_H__

#include <stdint.h>

#define ISP_METERING_ZONES_AE5_S                          4
#define ISP_METERING_ZONES_AE5_V                          33
#define ISP_METERING_ZONES_AE5_H                          33

/*****************************************************************************************
 *				  3ALib Register API			 	 	 *
 *****************************************************************************************/
typedef struct _modulation_entry_t {
    uint16_t x, y;
} modulation_entry_t;

typedef struct _ae_balanced_param_t {
    uint32_t pi_coeff;
    uint32_t target_point;
    uint32_t tail_weight;
    uint32_t long_clip;
    uint32_t er_avg_coeff;
    uint32_t hi_target_prc;
    uint32_t hi_target_p;
    uint32_t enable_iridix_gdg;
    uint32_t AE_tol;
} ae_balanced_param_t;

typedef struct _ae_misc_info_ {
    int32_t sensor_exp_number;

    int32_t total_gain;
    int32_t max_exposure_log2;
    uint32_t global_max_exposure_ratio;

    uint32_t iridix_contrast;
    uint32_t global_exposure;
    uint8_t global_ae_compensation;
    uint8_t global_manual_exposure;
    uint8_t global_manual_exposure_ratio;
    uint8_t global_exposure_ratio;
} ae_misc_info_t;

typedef struct _ae_calibration_data_ {
    uint8_t *ae_corr_lut;
    uint32_t ae_corr_lut_len;

    uint32_t *ae_exp_corr_lut;
    uint32_t ae_exp_corr_lut_len;

    modulation_entry_t *ae_hdr_target;
    uint32_t ae_hdr_target_len;

    modulation_entry_t *ae_exp_ratio_adjustment;
    uint32_t ae_exp_ratio_adjustment_len;
} ae_calibration_data_t;

typedef struct _ae_1024bin_weight_ {
        uint32_t zones_size;
        uint8_t zones_weight[ISP_METERING_ZONES_AE5_V * ISP_METERING_ZONES_AE5_H];
} ae_1024bin_weight_t;

typedef struct _ae_5bin_info_ {
        uint32_t zones_size;
        uint16_t zones_v;
        uint16_t zones_h;
        uint16_t threshold0_1;
        uint16_t threshold1_2;
        uint16_t threshold3_4;
        uint16_t threshold4_5;
        uint16_t normal_bin0;
        uint16_t normal_bin1;
        uint16_t normal_bin3;
        uint16_t normal_bin4;
} ae_5bin_info_t;

typedef struct _ae_out_info_ {
        uint32_t line[4];
        uint32_t line_num;
        uint32_t sensor_again[4];
        uint32_t sensor_again_num;
        uint32_t sensor_dgain[4];
        uint32_t sensor_dgain_num;
        uint32_t isp_dgain;
} ae_out_info_t;

typedef struct _ae_acamera_input_ {
    ae_balanced_param_t *ae_ctrl; 
    ae_misc_info_t misc_info;
    ae_calibration_data_t cali_data;
    ae_5bin_info_t ae_5bin_data;
    uint32_t ctx_id; 
} ae_acamera_input_t;

#endif
