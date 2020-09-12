/*
 *  Copyright (c) 2019 by Horizon
 * \file bpu_predict.h
 * \brief BPU predict API for Horizon BPU Platform.
 */

#ifndef BPU_INDEX_CONSIST_H_
#define BPU_INDEX_CONSIST_H_

#include <stdint.h>
#include <stdlib.h>

#include "bpu_predict.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

int BPU_runModelForIDXConsistency(BPUHandle handle,
                                  const char *model_name,
                                  BPUPyramidBuffer input,
                                  BPUBBox *bbox,
                                  int nBox,
                                  int *resizable_cnt,
                                  BPU_Buffer_Handle output[],
                                  int nOutput,
                                  BPUModelHandle *model_handle,
                                  bool do_normalize);

int BPU_getHBMhandleFromBPUhandle(BPUHandle handle, uint64_t *hbm_handle);

struct BPUCNNBuffer;

int BPU_runModelFromCNNBuffer(BPUHandle handle,
                              const char *model_name,
                              BPUCNNBuffer *input_y,
                              BPUCNNBuffer *input_uv,
                              int pyr_level,
                              BPU_Buffer_Handle output[],
                              int nOutput,
                              BPUModelHandle *model_handle,
                              BPU_Buffer_Handle *extra_input = nullptr,
                              int extra_input_size = 0,
                              int core_id = -1);
/*
 * construct a BPUCNN Buffer from exist buffer pointer
 */
BPUCNNBuffer *BPU_createBPUCNNBuffer(int width,
                                     int height,
                                     int channel,
                                     size_t lv_cnt);

/*
 * get pointer of BPU Buffer data. the return pointer is void*,
 * and can be cast to other data type
 */
void *BPU_getRawCNNBufferPtr(BPUCNNBuffer *buff, size_t lv, size_t *size);

int BPU_freeBPUCNNBuffer(BPUCNNBuffer *buff);

int BPU_runModelFromResizerBuffer(BPUHandle handle,
                                  const char *model_name,
                                  BPUCNNBuffer *input_y,
                                  BPUCNNBuffer *input_uv,
                                  BPUBBox *bbox,
                                  int nBox,
                                  int *resizable_cnt,
                                  BPU_Buffer_Handle output[],
                                  int nOutput,
                                  BPUModelHandle *model_handle,
                                  int core_id = -1);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // BPU_INDEX_CONSIST_H_
