/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream framework C interface
 * @author    shuhuan.sun
 * @email     shuhuan.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.4
 */

#ifndef TEST_INCLUDE_HOBOTXSTREAM_C_STRUCT_TRANSFER_BBOX_TRANSFER_H_
#define TEST_INCLUDE_HOBOTXSTREAM_C_STRUCT_TRANSFER_BBOX_TRANSFER_H_

#include <memory>

#include "./array_transfer.h"
#include "hobotxsdk/xstream_capi_type_helper.h"
#include "hobotxstream/c_data_types/bbox.h"
#include "hobotxstream/data_types/bbox.h"

namespace xstream {

XSTREAM_ARRAY_CHILD_TRANSFER(BBox);

}  // namespace xstream

#endif  // TEST_INCLUDE_HOBOTXSTREAM_C_STRUCT_TRANSFER_BBOX_TRANSFER_H_
