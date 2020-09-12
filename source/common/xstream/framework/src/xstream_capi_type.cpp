/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xsoul c framework interface
 * @author    shuhuan.sun
 * @email     shuhuan.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.14
 */

#include "hobotxsdk/xstream_capi_type.h"

#include <string>

#include "hobotxsdk/xstream_capi.h"
#include "hobotxsdk/xstream_capi_type_helper.h"
#include "hobotxsdk/xstream_data.h"

void HobotXStreamCapiDataFree(HobotXStreamCapiData** data) {
  if (data && *data) {
    auto context = reinterpret_cast<xstream::CppContext*>((*data)->context_);
    context->cfree_method_(data);
  }
}

HobotXStreamCapiDataList* HobotXStreamCapiDataListAlloc(size_t length) {
  auto datalist = reinterpret_cast<HobotXStreamCapiDataList*>(
      std::calloc(1, sizeof(HobotXStreamCapiDataList) +
                         sizeof(HobotXStreamCapiData*) * length));
  // std::calloc函数会保证申请的内存全为0
  datalist->datas_size_ = length;
  return datalist;
}

void HobotXStreamCapiDataListFree(HobotXStreamCapiDataList** datalist) {
  if (datalist && *datalist) {
    for (size_t i = 0; i < (*datalist)->datas_size_; ++i) {
      HobotXStreamCapiDataFree(&(*datalist)->datas_[i]);
    }
    free(*datalist);
    *datalist = nullptr;
  }
}

HobotXStreamCapiBaseDataVector* HobotXStreamCapiBaseDataVectorAlloc(
    size_t length) {
  XSTREAM_CAPI_BASE_ALLOC(BaseDataVector, data_vector);
  data_vector->datas_ = HobotXStreamCapiDataListAlloc(length);
  return data_vector;
}

void HobotXStreamCapiBaseDataVectorFree(
    HobotXStreamCapiBaseDataVector** data_vector) {
  if (data_vector && *data_vector) {
    XSTREAM_CAPI_BASE_FREE(BaseDataVector, data_vector);
    HobotXStreamCapiDataListFree(&((*data_vector)->datas_));
    free(*data_vector);
    *data_vector = nullptr;
  }
}

namespace xstream {

XSTREAM_DEFINE_2C_TRANSFER_FUNC(BaseDataVector, cpp_data) {
  XSTREAM_BASE_2C_TRANSFER_PROCESS(BaseDataVector, cpp_data, c_data,
                                   cpp_data->datas_.size());

  auto& in = cpp_data;
  auto& out = c_data;
  for (size_t i = 0; i < in->datas_.size(); ++i) {
    auto& in_data_i = in->datas_[i];
    out->datas_->datas_[i] = (in_data_i->c_data_)->cpp2c_method_(in_data_i);
  }

  return out;
}

XSTREAM_DEFINE_2CPP_TRANSFER_FUNC(BaseDataVector, c) {
  XSTREAM_BASE_2CPP_TRANSFER_PROCESS(BaseDataVector, c, cpp);
  auto& in = c;
  auto& out = cpp;
  out->datas_.resize(in->datas_->datas_size_);
  for (size_t i = 0; i < in->datas_->datas_size_; ++i) {
    auto& in_data_i = in->datas_->datas_[i];
    out->datas_[i] = reinterpret_cast<CppContext*>(in_data_i->context_)
                         ->c2cpp_method_(in_data_i);
  }
  return out;
}
}  // namespace xstream
