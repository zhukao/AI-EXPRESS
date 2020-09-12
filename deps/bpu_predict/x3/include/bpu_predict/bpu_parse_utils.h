/*
 *  Copyright (c) 2019 by Horizon
 * \file bpu_parse_utils.h
 * \brief BPU parse output utils API for Horizon BPU Platform.
 */

#ifndef BPU_PARSE_UTILS_H_
#define BPU_PARSE_UTILS_H_

#include "bpu_predict.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

struct BPUBBoxF32 {
  float left;
  float top;
  float right;
  float bottom;
  float score;
  float class_label;
};

struct BPUBBoxInt16 {
  int16_t left;
  int16_t top;
  int16_t right;
  int16_t bottom;
  int8_t score;
  uint8_t class_label;
  int16_t padding[3];
};

struct BPURppBBox {
  int bbox_num;
  enum { bbox_type_int16, bbox_type_f32 } result_type;
  BPUBBoxInt16 *bbox_ptr_int16;
  BPUBBoxF32 *bbox_ptr_f32;
};

/*
 * \brief parse det thresh result, get bbox
 */
int BPU_parseDetThreshResult(BPUHandle handle,
                             const char *model_name,
                             BPU_Buffer_Handle output[],
                             int nOutput,
                             BPUBBox **bbox,
                             int *nBox,
                             const char **cls_names,
                             int nCls);

/*
 * \brief parse channel max result, get classify index
 */
int BPU_parseChannelMaxResult(BPUHandle handle,
                              const char *model_name,
                              BPU_Buffer_Handle output[],
                              int nOutput,
                              int *result,
                              int nRes);

/*
 * \brief parse rpp op output result, it can get BPURppBBox structure
 * \containing bbox ptr; note that bbox result should be stored as soon
 * \as possible because memory pointed to by bbox ptr will be reused after
 * \calling BPU_releaseModelHandle and BPU_freeBPUBuffer.
 */
int BPU_parseRPPResult(BPUHandle handle,
                       const char *model_name,
                       BPU_Buffer_Handle output[],
                       int output_index,
                       BPURppBBox *rpp_bbox);

/*
 * \brief parse dpp op output result, get bbox
 */
int BPU_parseDPPResult(BPUHandle handle,
                       const char *model_name,
                       BPU_Buffer_Handle output[],
                       int nOutput,
                       BPUBBox **bbox,
                       int *nBox);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // end of BPU_PARSE_UTILS_H_
