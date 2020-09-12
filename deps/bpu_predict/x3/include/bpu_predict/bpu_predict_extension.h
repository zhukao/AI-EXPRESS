/*
 *  Copyright (c) 2019 by Horizon
 * \file bpu_predict.h
 * \brief BPU predict API for Horizon BPU Platform.
 */

#ifndef INCLUDE_BPU_PREDICT_BPU_PREDICT_EXTENSION_H_
#define INCLUDE_BPU_PREDICT_BPU_PREDICT_EXTENSION_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// typedef uint64_t size_t;

typedef struct hb_BPU_MEMORY_S {
  uint64_t phyAddr;
  void *virAddr;
  size_t memSize;
} BPU_MEMORY_S;

typedef enum hb_BPU_OP_TYPE_E {
  BPU_OP_TYPE_CONV,
  BPU_OP_TYPE_UNKNOWN,
} BPU_OP_TYPE_E;

typedef enum hb_BPU_LAYOUT_E {
  BPU_LAYOUT_NONE = 0,
  BPU_LAYOUT_NHWC,
  BPU_LAYOUT_NCHW,
} BPU_LAYOUT_E;

#define BPU_MODEL_MAX_SHAPE_DIM (8)
typedef struct hb_BPU_DATA_SHAPE_S {
  BPU_LAYOUT_E layout;
  int ndim;
  int d[BPU_MODEL_MAX_SHAPE_DIM];
} BPU_DATA_SHAPE_S;

typedef struct hb_BPU_RUN_CTRL_S {
  int core_id;
} BPU_RUN_CTRL_S;

typedef enum hb_BPU_DATA_TYPE_E {
  // IMG type is uint8
  BPU_TYPE_IMG_Y,
  BPU_TYPE_IMG_YUV_NV12,
  BPU_TYPE_IMG_YUV444,
  BPU_TYPE_IMG_BGR,
  BPU_TYPE_IMG_BGRP,
  BPU_TYPE_IMG_RGB,
  BPU_TYPE_IMG_RGBP,
  BPU_TYPE_IMG_NV12_SEPARATE,  // for separate yuv nv12
  BPU_TYPE_TENSOR_U8,          // for uint8 tensor
  BPU_TYPE_TENSOR_S8,          // for signed int8
  BPU_TYPE_TENSOR_F32,         // for float32
  BPU_TYPE_TENSOR_S32,         // for int32
  BPU_TYPE_TENSOR_U32,         // for uint32
  BPU_TYPE_MAX,
} BPU_DATA_TYPE_E;

typedef struct hb_BPU_MODEL_NODE_S {
  BPU_OP_TYPE_E op_type;  // only used for output node
  BPU_DATA_TYPE_E data_type;
  BPU_DATA_SHAPE_S shape;
  BPU_DATA_SHAPE_S aligned_shape;
  const char *name;
  uint8_t *shifts;  // only used for output node
  int shift_len;    // only used for output node
} BPU_MODEL_NODE_S;

typedef struct hb_BPU_MODEL_S {
  void *handle;
  int input_num;
  int output_num;
  BPU_MODEL_NODE_S *inputs;
  BPU_MODEL_NODE_S *outputs;
} BPU_MODEL_S;

typedef struct hb_BPU_TENSOR_S {
  BPU_DATA_TYPE_E data_type;
  BPU_DATA_SHAPE_S data_shape;
  BPU_DATA_SHAPE_S aligned_shape;
  BPU_MEMORY_S data;
  BPU_MEMORY_S data_ext;
} BPU_TENSOR_S;

typedef enum hb_BPU_GLOBAL_CONFIG_E {
  BPU_GLOBAL_CONFIG_MAX_TASK_NUM,
  BPU_GLOBAL_CONFIG_MAX_MEM_POOL_SIZE,
  BPU_GLOBAL_CONFIG_DEBUG,
  BPU_GLOBAL_CONV_MAPPING_FILE,
  BPU_GLOBAL_CONV_DUMP_PATH,
  BPU_GLOBAL_CONFIG_ACCOUNT_MEMORY_USAGE,
  BPU_GLOBAL_ENGINE_TYPE,
} BPU_GLOBAL_CONFIG_E;

/*
 * \brief load model from address
 * return 0 for everything is ok, error code for encounter a problem.
 */
int HB_BPU_loadModel(const void *model_data,
                     int model_size,
                     BPU_MODEL_S *model);

/*
 * \brief load model from file
 * return 0 for everything is ok, error code for encounter a problem.
 */
int HB_BPU_loadModelFromFile(const char *model_file_name, BPU_MODEL_S *model);

/*
 * \brief release model
 *  return 0 for everything is ok, error code for encounter a problem.
 */
int HB_BPU_releaseModel(BPU_MODEL_S *model);

/*
 * \brief get the version string of runtime lib
 *  return 0 for everything is ok, error code for encounter a problem.
 */
const char *HB_BPU_getVersion();

/*
 * \brief get error Name from error code.
 */
const char *HB_BPU_getErrorName(int error_code);

/*
 * \brief set global config by k-v way.
 */
int HB_BPU_setGlobalConfig(BPU_GLOBAL_CONFIG_E config_key,
                           const char *config_value);

typedef void *BPU_TASK_HANDLE;

/*
 * \brief there are two ways to run model, synchronous and asynchronous.
 * When in synchronous mode, calling this interface will block the current
 * thread until the model runs to completion. Otherwise, calling this interface
 * will return immediately, after which you should use HB_BPU_waitModelDone with
 * task_handle wait to the end.
 */
int HB_BPU_runModel(const BPU_MODEL_S *model,
                    const BPU_TENSOR_S input_data[],
                    const int input_num,
                    const BPU_TENSOR_S output_data[],
                    const int output_num,
                    const BPU_RUN_CTRL_S *run_ctrl,
                    bool is_sync,
                    BPU_TASK_HANDLE *task_handle);

typedef struct hb_BPU_BBOX {
  float x1;
  float y1;
  float x2;
  float y2;
  float score;
  int type;
  bool resizable;
} BPU_BBOX;
typedef void *BPU_CAMERA_BUFFER;

/*
 * \brief run mode with bbox, there are two ways to run model, synchronous and
 * asynchronous. When in synchronous mode, calling this interface will block the
 * current thread until the model runs to completion. Otherwise, calling this
 * interface will return immediately, after which you should use
 * HB_BPU_waitModelDone with task_handle wait to the end.
 */
int HB_BPU_runModelWithBbox(const BPU_MODEL_S *model,
                            BPU_CAMERA_BUFFER input,
                            BPU_BBOX bbox[],
                            int nBox,
                            const BPU_TENSOR_S output_data[],
                            int output_num,
                            const BPU_RUN_CTRL_S *run_ctrl,
                            bool is_sync,
                            int *resizable_cnt,
                            BPU_TASK_HANDLE *task_handle);

/*
 * \brief wait for the run to end when in asynchronous mode.
 */
int HB_BPU_waitModelDone(BPU_TASK_HANDLE *task_handle);

/*
 * \brief release task when in asynchronous mode.
 */
int HB_BPU_releaseTask(BPU_TASK_HANDLE *task_handle);

/*
 * \brief create a group on bpu
 * \return group id.
 */
int HB_BPU_createGroup();

/*
 * \brief set model to group
 * when you do not want model belong to some group, can set group id to zero.
 * group id is zero means the default group.
 */
int HB_BPU_setModelGroup(BPU_MODEL_S *model, int group_id);

/*
 * \brief set proportion of group. The proportion must in range [0, 100], and is
 * calculated as proportion / 100
 */
int HB_BPU_setGroupProportion(int group_id, int proportion);

/*
 * \brief delete a group id
 */
int HB_BPU_deleteGroup(int group_id);

// memory api
/*
 * \brief alloc bpu memory for input and output of model,
 * flag cachable indicates whether a cache is required.
 */
int HB_SYS_bpuMemAlloc(const char *name,
                       size_t alloc_mem_size,
                       bool cachable,
                       BPU_MEMORY_S *mem);

#define HB_SYS_MEM_CACHE_INVALIDATE (1)  // flush memory to CPU
#define HB_SYS_MEM_CACHE_CLEAN (2)       // flush CPU to memory

/*
 * \brief flush cache with flag.
 * HB_SYS_MEM_CACHE_INVALIDATE should be used  after write,
 * and HB_SYS_MEM_CACHE_CLEAN should be used before read .
 */
int HB_SYS_flushMemCache(const BPU_MEMORY_S *mem, int flag);

/*
 * \brief query memory whether is cachable or not.
 */
int HB_SYS_isMemCachable(const BPU_MEMORY_S *mem);

/*
 * \brief free bpu mem.
 */
int HB_SYS_bpuMemFree(BPU_MEMORY_S *mem);

/*
 * \brief get memory info from virtual address pointer.
 *        if it point to a bpu memory
 */
int HB_SYS_getMemInfo(const void *virAddr, uint64_t *phyAddr, int *is_cachable);

/*
 * \brief cast vir addr to phy addr.
 */
int HB_SYS_virAddrAlloc(void **virAddr,
                        const uint64_t *phyAddr,
                        size_t alloc_mem_size);

/*
 * \brief free vir addr.
 */
int HB_SYS_virAddrFree(const void *virAddr);

/**
 * \brief set model prior, model can run in prior
 *        queue when it is set
 */
int HB_BPU_setModelPrior(BPU_MODEL_S *model);

typedef struct hb_BPU_ADDR_INFO_S {
  uint16_t width;
  uint16_t height;
  uint16_t step;
  uint64_t y_paddr;
  uint64_t c_paddr;
  uint64_t y_vaddr;
  uint64_t c_vaddr;
} BPU_ADDR_INFO_S;

// for ds|us
#define DOWN_SCALE_MAIN_MAX 6
#define DOWN_SCALE_MAX 24
#define UP_SCALE_MAX 6

typedef struct hb_BPU_IMG_INFO_S {
  int slot_id;        // getted slot buff
  int frame_id;       // for x2 may be 0 - 0xFFFF or 0x7FFF
  int64_t timestamp;  // BT from Hisi; mipi & dvp from kernel time
  int img_format;     // now only support yuv420sp
  int ds_pym_layer;   // get down scale layers
  int us_pym_layer;   // get up scale layers
  BPU_ADDR_INFO_S *down_scale[DOWN_SCALE_MAX];
  BPU_ADDR_INFO_S *up_scale[UP_SCALE_MAX];
  BPU_ADDR_INFO_S *down_scale_main[DOWN_SCALE_MAIN_MAX];
  int cam_id;
} BPU_IMG_INFO_S;

typedef struct hb_BPU_PYRAMID_RESULT_S {
  BPU_IMG_INFO_S result_info;
} BPU_PYRAMID_RESULT_S;

typedef struct hb_BPU_CAMERA_IMAGE_INFO_S {
  int frame_id;       // for x2 may be 0 - 0xFFFF or 0x7FFF
  int64_t timestamp;  // BT from Hisi; mipi & dvp from kernel time
  int img_format;     // now only support yuv420sp
  BPU_ADDR_INFO_S src_img;
  int cam_id;
} BPU_CAMERA_IMAGE_INFO_S;

/*
 * \brief convert layout.
 */

typedef enum hb_BPU_CONVERT_LAYOUT_METHOD_E {
  BPU_1HW1_CONVERT_METHOD = 0,
  BPU_111C_CONVERT_METHOD,
  BPU_1111_CONVERT_METHOD,
  BPU_NHWC_CONVERT_METHOD
} BPU_CONVERT_LAYOUT_METHOD_E;
/*
 * \brief convert layout.
 */
int HB_BPU_convertLayout(const BPU_MODEL_S *model,
                         void *to_data,
                         const void *from_data,
                         BPU_CONVERT_LAYOUT_METHOD_E convert_method,
                         uint32_t output_index,
                         uint32_t n_index,
                         uint32_t h_index,
                         uint32_t w_index,
                         uint32_t c_index);

typedef enum hb_BPU_RESIZE_TYPE_E {
  BPU_RESIZE_TYPE_BILINEAR
} BPU_RESIZE_TYPE_E;

typedef struct hb_BPU_RESIZE_CTRL_S {
  BPU_RESIZE_TYPE_E resize_type;
  BPU_DATA_TYPE_E output_type;  // set output type.
  int core_id;
} BPU_RESIZE_CTRL_S;

typedef struct hb_BPU_ROI_S {
  int x1;
  int y1;
  int x2;
  int y2;
} BPU_ROI_S;

/*
 * \brief resize input image, layout of result is 8w4c with padding.
 */
int HB_BPU_resize(const BPU_TENSOR_S *src,
                  BPU_TENSOR_S *dest,
                  const BPU_RESIZE_CTRL_S *ctrl_param);

/*
 * \brief roi resize, layout of result is 8w4c with padding.
 */
int HB_BPU_cropAndResize(const BPU_TENSOR_S *src,
                         const BPU_ROI_S *input_roi,
                         BPU_TENSOR_S *dest,
                         const BPU_RESIZE_CTRL_S *ctrl_param);

/*
 * \brief get nhwc layout of resize result.
 */
int HB_BPU_getResizeResultWithoutPadding(const BPU_MEMORY_S *src,
                                         const BPU_DATA_TYPE_E data_type,
                                         const BPU_DATA_SHAPE_S *shape,
                                         void *dest,
                                         const int dest_size);

int HB_BPU_getImageAlignedShape(const BPU_DATA_SHAPE_S *shape,
                                int *aligned_size);

/*
 * \brief get Height,Width,Channel index within dimension according to data_type
 * and layout.If data_type is image, layout parameter can be null, otherwise
 * layout parameter is required.
 */
int HB_BPU_getHWCIndex(BPU_DATA_TYPE_E data_type,
                       const BPU_LAYOUT_E *layout,
                       int *h_idx,
                       int *w_idx,
                       int *c_idx);

/*
 * \brief get Height size,Width size within shape's dimension according to
 * data_type.
 */
int HB_BPU_getHW(BPU_DATA_TYPE_E data_type,
                 const BPU_DATA_SHAPE_S *shape,
                 int *height,
                 int *width);

/*
 * \brief: get memory usage detail after HB_BPU_loadModel
 */
int HB_BPU_getMemoryUsage(BPU_MODEL_S *model,
                          size_t *bpu_peak_memory_usage,
                          size_t *bpu_memory_occupation,
                          size_t *cpu_peak_memory_usage,
                          size_t *cpu_memory_occupation);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // INCLUDE_BPU_PREDICT_BPU_PREDICT_EXTENSION_H_
