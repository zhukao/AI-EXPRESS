/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream framework C interface
 * @author    shuhuan.sun
 * @email     shuhuan.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.4
 */

#ifndef HOBOTXSTREAM_C_DATA_TYPES_BBOX_H_
#define HOBOTXSTREAM_C_DATA_TYPES_BBOX_H_

#include "array.h"
#include "hobotxsdk/xstream_capi_type.h"

#ifdef __cplusplus
extern "C" {
#endif

XSTREAM_CAPI_INHERIT_FROM_ARRAY(BBox, 4)

#ifdef __cplusplus
}
#endif

#endif  // HOBOTXSTREAM_C_DATA_TYPES_BBOX_H_
