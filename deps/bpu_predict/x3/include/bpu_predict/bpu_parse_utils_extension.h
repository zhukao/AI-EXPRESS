/*
 *  Copyright (c) 2019 by Horizon
 * \file bpu_parse_utils.h
 * \brief BPU parse output utils API for Horizon BPU Platform.
 */

#ifndef BPU_PARSE_UTILS_EXTENSIONH_
#define BPU_PARSE_UTILS_EXTENSIONH_

#include "bpu_predict_extension.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct hb_BPU_BBOX_F32 {
  float left;
  float top;
  float right;
  float bottom;
  float score;
  float class_label;
} BPU_BBOX_F32;

typedef struct hb_BPU_BBOX_INT16 {
  int16_t left;
  int16_t top;
  int16_t right;
  int16_t bottom;
  int8_t score;
  uint8_t class_label;
  int16_t padding[3];
} BPU_BBOX_INT16;

typedef struct hb_BPU_RPP_BBOX {
  int bbox_num;
  enum { bbox_type_int16, bbox_type_f32 } result_type;
  BPU_BBOX_INT16 *bbox_ptr_int16;
  BPU_BBOX_F32 *bbox_ptr_f32;
} BPU_RPP_BBOX;

/*
 * \brief parse det thresh result, get bbox
 */
int HB_BPU_parseDetThreshResult(const BPU_MODEL_S *model,
                                const BPU_TENSOR_S output_data[],
                                int output_num,
                                const char **class_names,
                                int class_num,
                                BPU_BBOX **bbox,
                                int *bbox_num);

/*
 * \brief parse channel max result, get classify index
 */
int HB_BPU_parseChannelMaxResult(const BPU_MODEL_S *model,
                                 const BPU_TENSOR_S output_data[],
                                 int output_num,
                                 int result_num,
                                 int *result);

/*
 * \brief parse rpp op output result, it can get BPURppBBox structure
 * \containing bbox ptr; note that bbox result should be stored as soon
 * \as possible because memory pointed to by bbox ptr will be reused after
 * \calling BPU_releaseModelHandle and BPU_freeBPUBuffer.
 */
int HB_BPU_parseRPPResult(const BPU_MODEL_S *model,
                          const BPU_TENSOR_S output_data[],
                          int output_index,
                          BPU_RPP_BBOX *rpp_bbox);

/*
 * \brief parse dpp op output result, get bbox
 */
int HB_BPU_parseDPPResult(const BPU_MODEL_S *model,
                          const BPU_TENSOR_S output_data[],
                          int output_num,
                          BPU_BBOX **bbox,
                          int *bbox_num);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // end of BPU_PARSE_UTILS_EXTENSIONH_
