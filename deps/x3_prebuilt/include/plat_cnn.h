/*************************************************************************
 *                     COPYRIGHT NOTICE
 *            Copyright 2016-2018 Horizon Robotics, Inc.
 *                   All rights reserved.
 *************************************************************************/

#ifndef __PLAT_CNN_H__
#define __PLAT_CNN_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

//////////////////////////CNN Core API//////////////////////////
enum cnn_start_method {
	START_FROM_UNKNOWN,
	START_FROM_DDR,
	START_FROM_PYM,
	START_FROM_RESIZER,
	START_METHOD_NUM
};

enum cnn_core_type {
	CORE_TYPE_UNKNOWN,
	CORE_TYPE_4PE,
	CORE_TYPE_1PE,
	CORE_TYPE_2PE,
	CORE_TYPE_ANY,
	CORE_TYPE_INVALID,
};

typedef struct {
	int32_t left;
	int32_t top;
	int32_t right;
	int32_t bottom;

	/* origin image info */
	uint64_t y_ddr_base_addr;
	uint64_t uv_ddr_base_addr;
	int32_t img_w;
	int32_t img_h;
	int32_t img_w_stride;

	/* resizer y/uv data or not */
	int32_t y_en;
	int32_t uv_en;

	/* resize dst size */
	int32_t dst_w;
	int32_t dst_h;

	/* other set for resizer*/
	uint32_t padding_mode;
	uint64_t extra_params[4];
} roi_box_t;

typedef void (*fc_done_cb)(uint32_t id, int errno);

int cnn_core_num(void);
int cnn_core_type(unsigned int core_index);
int cnn_core_open(unsigned int core_index);
void cnn_core_close(unsigned int core_index);

int cnn_core_set_fc(void *fc, int num, unsigned int core_index, fc_done_cb done_cb);
int cnn_core_set_fc_with_rsz(void *fc, int num, unsigned int core_index, fc_done_cb done_cb, roi_box_t *boxes, int boxes_num);

int cnn_core_fc_avl_id(unsigned int core_index);
int cnn_core_fc_all_cap(unsigned int core_index);
int cnn_core_fc_avl_cap(unsigned int core_index);
int cnn_core_wait_fc_done(unsigned int core_index, int timeout);
int cnn_core_check_fc_done(unsigned int core_index, uint32_t id, int timeout);

int cnn_set_group_proportion(int group_id, int proportion);
int cnn_core_set_fc_group(void *fc, int num, unsigned int core_index, fc_done_cb done_cb, int group_id);
int cnn_core_set_fc_with_rsz_group(void *fc, int num, unsigned int core_index, fc_done_cb done_cb, roi_box_t *boxes, int boxes_num, int group_id);

/*"hb_" start APIs*/
int hb_bpu_core_num(void);
int hb_bpu_core_type(unsigned int core_index);
int hb_bpu_core_open(unsigned int core_index);
void hb_bpu_core_close(unsigned int core_index);

int hb_bpu_core_set_fc(void *fc, int num,
		unsigned int core_index, fc_done_cb done_cb);
int hb_bpu_core_set_fc_with_rsz(void *fc,
		int num, unsigned int core_index,
		fc_done_cb done_cb, roi_box_t *boxes, int boxes_num);

int hb_bpu_core_fc_avl_id(unsigned int core_index);
int hb_bpu_core_fc_all_cap(unsigned int core_index);
int hb_bpu_core_fc_avl_cap(unsigned int core_index);
int hb_bpu_core_wait_fc_done(unsigned int core_index, int timeout);
int hb_bpu_core_check_fc_done(unsigned int core_index, uint32_t id, int timeout);

int hb_bpu_set_group_proportion(int group_id, int proportion);
int hb_bpu_core_set_fc_group(void *fc, int num,
		unsigned int core_index, fc_done_cb done_cb, int group_id);
int hb_bpu_core_set_fc_with_rsz_group(void *fc, int num,
		unsigned int core_index, fc_done_cb done_cb,
		roi_box_t *boxes, int boxes_num, int group_id);
////////////////////////////////////////////////////////////////

//////////////////////////Memory API////////////////////////////
#define bpu_addr_t uint64_t
#define BPU_NON_CACHEABLE (0)
#define BPU_CACHEABLE (1)
#define BPU_CORE0 (0x10000)
#define BPU_CORE1 (0x20000)
#define ARM_TO_CNN (0)
#define CNN_TO_ARM (1)
#define BPU_MEM_INVALIDATE (1)
#define BPU_MEM_CLEAN (2)

bpu_addr_t bpu_mem_register(void *phy_addr, int size);
void bpu_mem_unregister(bpu_addr_t addr);

bpu_addr_t bpu_mem_alloc(int size, int flag);
bpu_addr_t bpu_cpumem_alloc(int size, int flag);
bpu_addr_t bpu_mem_alloc_with_label(int size, int flag, const char* label);
bpu_addr_t bpu_cpumem_alloc_with_label(int size, int flag, const char* label);
void bpu_mem_free(bpu_addr_t ptr);
void bpu_cpumem_free(bpu_addr_t ptr);
int bpu_memcpy(bpu_addr_t dst_addr, bpu_addr_t src_addr, unsigned size, int direction);
void bpu_mem_cache_flush(bpu_addr_t ptr, int size, int flag);
int bpu_mem_is_cacheable(bpu_addr_t addr);
uint64_t bpu_mem_phyaddr(bpu_addr_t addr);

/*"hb_" start APIs*/
bpu_addr_t hb_bpu_mem_register(void *phy_addr, int size);
void hb_bpu_mem_unregister(bpu_addr_t addr);

bpu_addr_t hb_bpu_mem_alloc(int size, int flag);
bpu_addr_t hb_bpu_cpumem_alloc(int size, int flag);
bpu_addr_t hb_bpu_mem_alloc_with_label(int size, int flag, const char* label);
bpu_addr_t hb_bpu_cpumem_alloc_with_label(int size, int flag, const char* label);
void hb_bpu_mem_free(bpu_addr_t ptr);
void hb_bpu_cpumem_free(bpu_addr_t ptr);
int hb_bpu_memcpy(bpu_addr_t dst_addr,
		bpu_addr_t src_addr, unsigned size, int direction);
void hb_bpu_mem_cache_flush(bpu_addr_t ptr, int size, int flag);
int hb_bpu_mem_is_cacheable(bpu_addr_t addr);
uint64_t hb_bpu_mem_phyaddr(bpu_addr_t addr);
////////////////////////////////////////////////////////////////

//////////////////////// Hardware oprations API////////////////
#define BPU_HIGHEST_FRQ  0
#define BPU_POWER_OFF    0
#define BPU_POWER_ON    1
#define BPU_CLK_OFF      0
#define BPU_CLK_ON      1

int bpu_set_power(int core_index, unsigned int status);
int bpu_set_frq_level(int core_index, int level);
int bpu_set_clk(int core_index, unsigned int status);
int bpu_get_total_level(unsigned int core_index);
int bpu_get_cur_level(unsigned int core_index);

/*"hb_" start APIs*/
int hb_bpu_set_power(int core_index, unsigned int status);
int hb_bpu_set_frq_level(int core_index, int level);
int hb_bpu_set_clk(int core_index, unsigned int status);
int hb_bpu_get_total_level(unsigned int core_index);
int hb_bpu_get_cur_level(unsigned int core_index);

#ifdef __cplusplus
}
#endif
#endif
