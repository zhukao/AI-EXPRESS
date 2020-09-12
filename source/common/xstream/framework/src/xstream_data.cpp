/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief provides base data struct for xsoul framework
 * @author    chuanyi.yang
 * @email     chuanyi.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.11.21
 */

#include "hobotxsdk/xstream_data.h"

#include "hobotxsdk/xstream_capi_type_helper.h"

namespace xstream {
BaseDataVector::BaseDataVector() {
  type_ = "BaseDataVector";
#ifdef C_INTERFACE
  XSTREAM_BASE_CPP_CONTEXT_INIT(BaseDataVector, c_data_);
#endif
}

BaseData::BaseData() {
  type_ = "BaseData";
  c_data_.reset(new xstream::CContext());
#ifdef C_INTERFACE
  XSTREAM_BASE_CPP_CONTEXT_INIT(BaseData, c_data_);
#endif
}

BaseData::~BaseData() {
}
}  // namespace xstream
