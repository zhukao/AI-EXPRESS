/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream framework C interface
 * @author    shuhuan.sun
 * @email     shuhuan.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.4
 */

#ifndef HOBOTXSTREAM_C_DATA_TYPES_ARRAY_H_
#define HOBOTXSTREAM_C_DATA_TYPES_ARRAY_H_

#include <stdint.h>
#include <stdlib.h>
#include "hobotxsdk/xstream_capi_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HobotXStreamCapiArray_ {
  HobotXStreamCapiBaseData parent_;
  uint32_t class_;      //< 分类信息
  float score_;         //< 置信度
  float* values_;       //< 数据指针
  size_t values_size_;  //< 数据长度
} HobotXStreamCapiArray;

HOBOT_EXPORT
HobotXStreamCapiArray* HobotXStreamCapiArrayAlloc();

HOBOT_EXPORT
void HobotXStreamCapiArrayFree(HobotXStreamCapiArray** array);

#define XSTREAM_CAPI_INHERIT_FROM_ARRAY(ChildType, size)                           \
  typedef HobotXStreamCapiArray HobotXStreamCapi##ChildType;                          \
  inline HobotXStreamCapi##ChildType* HobotXStreamCapi##ChildType##Alloc() {          \
    HobotXStreamCapi##ChildType* c_data = HobotXStreamCapiArrayAlloc();               \
    c_data->values_size_ = size;                                                \
    c_data->values_ = (float*)calloc(size, sizeof(float));                      \
    c_data->parent_.type_ = #ChildType;                                         \
    return c_data;                                                              \
  }                                                                             \
  inline void HobotXStreamCapi##ChildType##Free(HobotXStreamCapi##ChildType** data) { \
    free((*data)->values_);                                                     \
    HobotXStreamCapiArrayFree(data);                                               \
  }

#ifdef __cplusplus
}
#endif

#endif  // HOBOTXSTREAM_C_DATA_TYPES_ARRAY_H_
