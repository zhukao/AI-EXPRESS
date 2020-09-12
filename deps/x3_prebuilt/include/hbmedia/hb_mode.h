#ifndef X3_COMMOM_GRP_HB_MODE_H_
#define X3_COMMOM_GRP_HB_MODE_H_

typedef struct {
	int isp_enable;
	int isp_flyby;
	int isp_dma_enable;
	int sif_ddr_in;
	int sif_ddr_out;
	int ipu_flyby;
	int ipu_source;
} _SYS_VIN_VPS_MODE_S;

typedef enum HB_SYS_VIN_VPS_MODE_E {
	VIN_ONLINE_VPS_ONLINE,
	VIN_ONLINE_VPS_OFFLINE,
	VIN_OFFLINE_VPS_ONLINE,   //  2
	VIN_OFFLINE_VPS_OFFINE,   //  3
	VIN_SIF_VPS_ONLINE,    //  4
	VIN_SIF_OFFLINE_ISP_OFFLINE_VPS_ONLINE,    //  5
	VIN_SIF_ONLINE_DDR_ISP_DDR_VPS_ONLINE,    //  6
	VIN_SIF_ONLINE_DDR_ISP_ONLINE_VPS_ONLINE,    //  7
	VIN_FEEDBACK_ISP_ONLINE_VPS_ONLINE,   // 8
	VIN_SIF_OFFLINE_VPS_OFFLINE,       //  9
	VIN_SIF_OFFLINE,                 //   10
} SYS_VIN_VPS_MODE_E;

int _sys_get_init_state(void);
int _sys_get_sif_ddr_input(int idx);
int _sys_get_sif_ddr_output(int idx);
int _sys_get_isp_enable(int idx);
int _sys_get_isp_dma_enable(int idx);
int _sys_get_isp_flyby(int idx);
int _sys_get_ipu_flyby(int idx);
int _sys_get_ipu_source(int idx);

int vin_vps_mode_set(int pipeId, const SYS_VIN_VPS_MODE_E mode);
int vin_vps_mode_get(int pipeId);


#endif  // X3_COMMOM_GRP_HB_MODE_H_
