/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream framework test Data
 * @author    ronghui.zhang
 * @email     ronghui.zhang@horizon.ai
 * @version   0.0.0.1
 * @date      2019.11.28
 */

#include "hobotxstream/data_types/orderdata.h"

#include <stdlib.h>

#include <memory>

namespace xstream {

OrderData::OrderData(size_t sequece_id) {
  type_ = "OrderData";
  sequence_id = sequence_id;
}

OrderData::~OrderData() {}
}  // namespace xstream
