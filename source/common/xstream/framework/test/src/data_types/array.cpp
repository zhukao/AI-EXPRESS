/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream framework C interface
 * @author    shuhuan.sun
 * @email     shuhuan.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.4
 */

#include "hobotxstream/c_data_types/array.h"

#include <stdlib.h>

#include <memory>

#include "hobotxstream/c_struct_transfer/array_transfer.h"
#include "hobotxsdk/xstream_capi_type_helper.h"

XSTREAM_DEFINE_DATA_FREE_PARENT(Array)

HobotXStreamCapiArray* HobotXStreamCapiArrayAlloc() {
  XSTREAM_CAPI_BASE_ALLOC(Array, array);
  return array;
}

void HobotXStreamCapiArrayFree(HobotXStreamCapiArray** array) {
  if (*array) {
    XSTREAM_CAPI_BASE_FREE(Array, array);
    free(*array);
    *array = nullptr;
  }
}

namespace xstream {

Array::Array(size_t values_size, float* values) : values_(values_size) {
  type_ = "Array";
  XSTREAM_BASE_CPP_CONTEXT_INIT(Array, c_data_);
  if (values) values_.assign(values, values + values_size);
}

Array::~Array() {}

}  // namespace xstream
