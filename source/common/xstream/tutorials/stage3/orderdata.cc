/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      order.h
 * @brief     Order class
 * @author    Qingpeng Liu (qingpeng.liu@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-02-24
 */
#include <stdlib.h>
#include <memory>

#include "xstream/tutorials/stage3/orderdata.h"

namespace xstream {

OrderData::OrderData(size_t sequece_id) {
    type_ = "OrderData";
    sequence_id = sequence_id;
}

OrderData::~OrderData() {
}
}  // namespace xstream

