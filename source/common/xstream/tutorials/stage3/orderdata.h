/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      order.h
 * @brief     Order class
 * @author    Qingpeng Liu (qingpeng.liu@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-02-24
 */
#ifndef XSTREAM_TUTORIALS_STAGE3_ORDERDATA_H_
#define XSTREAM_TUTORIALS_STAGE3_ORDERDATA_H_

#include <cstdint>
#include <cstdlib>
#include <vector>
#include "hobotxsdk/xstream_data.h"

namespace xstream {
struct OrderData : public xstream::BaseData {
    explicit  OrderData(size_t sequece_id);
    ~OrderData();

    size_t sequence_id = 0;
};
}  // namespace xstream

#endif  // XSTREAM_TUTORIALS_STAGE3_ORDERDATA_H_

