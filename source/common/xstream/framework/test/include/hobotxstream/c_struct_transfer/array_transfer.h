/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream framework C interface
 * @author    shuhuan.sun
 * @email     shuhuan.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.4
 */

#ifndef TEST_INCLUDE_HOBOTXSTREAM_C_STRUCT_TRANSFER_ARRAY_TRANSFER_H_
#define TEST_INCLUDE_HOBOTXSTREAM_C_STRUCT_TRANSFER_ARRAY_TRANSFER_H_

#include <memory>

#include "hobotxsdk/xstream_capi_type_helper.h"
#include "hobotxstream/c_data_types/array.h"
#include "hobotxstream/data_types/array.h"

namespace xstream {

XSTREAM_DECLEAR_TRANSFER(Array);

#define XSTREAM_ARRAY_CHILD_TRANSFER(ChildType)                            \
  inline XSTREAM_DEFINE_2C_TRANSFER_FUNC(ChildType, cpp_data) {            \
    XSTREAM_BASE_2C_TRANSFER_PROCESS(ChildType, cpp_data, c_data);         \
                                                                           \
    auto& in = cpp_data;                                                   \
    auto& out = c_data;                                                    \
    out->values_ = &(in->values_[0]);                                      \
    out->values_size_ = in->values_.size();                                \
    out->class_ = in->class_;                                              \
    out->score_ = in->score_;                                              \
                                                                           \
    return out;                                                            \
  }                                                                        \
                                                                           \
  inline XSTREAM_DEFINE_2CPP_TRANSFER_FUNC(ChildType, c) {                 \
    XSTREAM_BASE_2CPP_TRANSFER_PROCESS(ChildType, c, cpp, c->values_size_, \
                                       c->values_);                        \
    auto& in = c;                                                          \
    auto& out = cpp;                                                       \
    out->class_ = in->class_;                                              \
    out->score_ = in->score_;                                              \
                                                                           \
    return out;                                                            \
  }

XSTREAM_ARRAY_CHILD_TRANSFER(Array)

}  // namespace xstream

#endif  // TEST_INCLUDE_HOBOTXSTREAM_C_STRUCT_TRANSFER_ARRAY_TRANSFER_H_
