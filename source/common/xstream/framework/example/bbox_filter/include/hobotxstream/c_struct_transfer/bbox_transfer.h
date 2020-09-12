/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream framework C interface
 * @author    shuhuan.sun
 * @email     shuhuan.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.4
 */

#ifndef HOBOTXSTREAM_C_STRUCT_TRANSFER_BBOX_TRANSFER_H_
#define HOBOTXSTREAM_C_STRUCT_TRANSFER_BBOX_TRANSFER_H_

#include <memory>

#include "array_transfer.h"
#include "hobotxstream/c_data_types/bbox.h"
#include "hobotxstream/data_types/bbox.h"
#include "hobotxsdk/xstream_capi_type_helper.h"

namespace xstream {

XSTREAM_ARRAY_CHILD_TRANSFER(BBox);

}  // namespace xstream

#endif  // HOBOTXSTREAM_C_STRUCT_TRANSFER_BBOX_TRANSFER_H_
