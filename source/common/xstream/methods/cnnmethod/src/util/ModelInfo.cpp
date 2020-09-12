/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: ModelInfo.cpp
 * @Brief: definition of the ModelInfo
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-05-12 14:27:05
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-05-12 16:17:08
 */

#include "CNNMethod/util/ModelInfo.h"
#include "bpu_predict/bpu_internal.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

std::ostream &operator<<(std::ostream &os, const ModelInfo &info) {
  // output_layer_size
  os << "output layer size:" << std::endl;
  for (auto &o_size : info.output_layer_size_) {
    os << o_size << " ";
  }
  os << std::endl;
  // mxnet_output_layer_size
  os << "mxnet output layer size:" << std::endl;
  for (auto &o_size : info.mxnet_output_layer_size_) {
    os << o_size << " ";
  }
  os << std::endl;
  // aligned_nhwc
  os << "aligned nhwc:" << std::endl;
  for (auto &layer_nhwc : info.aligned_nhwc_) {
    for (auto &nhwc : layer_nhwc) {
      os << nhwc << " ";
    }
    os << std::endl;
  }

  // real nhwc
  os << "real nhwc:" << std::endl;
  for (auto &layer_nhwc : info.real_nhwc_) {
    for (auto &nhwc : layer_nhwc) {
      os << nhwc << " ";
    }
    os << std::endl;
  }

  // shift
  os << "output layout shift:" << std::endl;
  for (auto &layout_shift : info.all_shift_) {
    for (auto &shift : layout_shift) {
      os << shift << " ";
    }
    os << std::endl;
  }
  // elem_size
  os << "elem_size:" << std::endl;
  for (auto &elem_size : info.elem_size_) {
    os << elem_size << " ";
  }
  os << std::endl;

  // input hnwc
  os << "input_nhwc:" << std::endl;
  for (auto v : info.input_nhwc_) {
    os << v << " ";
  }
  os << std::endl;

  // aligned input hnwc
  os << "aligned_input_nhwc:" << std::endl;
  for (auto v : info.aligned_input_nhwc_) {
    os << v << " ";
  }
  os << std::endl;
  return os;
}


void ModelInfo::Init(BPU_MODEL_S *bpu_model, int data_type) {
  LOGD << "data_type: " << data_type;
  if (data_type != -1) {  // 同构模型 TODO(zhe.sun) 异构模型
    data_type_ = BPU_DATA_TYPE_E(data_type);
  }
  auto out_layer_num = bpu_model->output_num;
  aligned_nhwc_.resize(out_layer_num);
  real_nhwc_.resize(out_layer_num);
  output_layer_size_.resize(out_layer_num);
  mxnet_output_layer_size_.resize(out_layer_num);
  all_shift_.resize(out_layer_num);

  // get info
  for (int i = 0; i < out_layer_num; ++i) {
    int h_idx, w_idx, c_idx;
    HB_BPU_getHWCIndex(bpu_model->outputs[i].data_type,
                       &bpu_model->outputs[i].shape.layout,
                       &h_idx, &w_idx, &c_idx);
    // aligned_nhwc_, real_nhwc_
    {
      aligned_nhwc_[i].push_back(bpu_model->outputs[i].aligned_shape.d[0]);
      aligned_nhwc_[i].push_back(bpu_model->outputs[i].aligned_shape.d[h_idx]);
      aligned_nhwc_[i].push_back(bpu_model->outputs[i].aligned_shape.d[w_idx]);
      aligned_nhwc_[i].push_back(bpu_model->outputs[i].aligned_shape.d[c_idx]);
      real_nhwc_[i].push_back(bpu_model->outputs[i].shape.d[0]);
      real_nhwc_[i].push_back(bpu_model->outputs[i].shape.d[h_idx]);
      real_nhwc_[i].push_back(bpu_model->outputs[i].shape.d[w_idx]);
      real_nhwc_[i].push_back(bpu_model->outputs[i].shape.d[c_idx]);
    }
    // all_shift_
    for (unsigned int k = 0; k < real_nhwc_[i][c_idx]; k++) {
      all_shift_[i].push_back(bpu_model->outputs[i].shifts[k]);
    }

    int model_out_size = 1;
    int mxnet_model_out_size = 1;
    for (size_t j = 0; j < aligned_nhwc_[i].size(); j++) {
      model_out_size *= aligned_nhwc_[i][j];
    }
    for (size_t j = 0; j < real_nhwc_[i].size(); j++) {
      mxnet_model_out_size *= real_nhwc_[i][j];
    }

    // TODO(zhe.sun) 默认输出float
    // int out_type_size = sizeof(int8_t);
    int out_type_size = sizeof(float);
    // elem_size_, output_layer_size_, mxnet_output_layer_size_
    elem_size_.push_back(out_type_size);
    output_layer_size_[i] = out_type_size * model_out_size;
    mxnet_output_layer_size_[i] = out_type_size * mxnet_model_out_size;
  }

  // get info: input_nhwc_, aligned_input_nhwc_
  HOBOT_CHECK(bpu_model->input_num == 1)
      << "model input_layer: " << bpu_model->input_num;
  int h_idx, w_idx, c_idx;
    HB_BPU_getHWCIndex(bpu_model->inputs[0].data_type,
                       &bpu_model->inputs[0].shape.layout,
                       &h_idx, &w_idx, &c_idx);
  input_nhwc_.push_back(bpu_model->inputs[0].shape.d[0]);
  input_nhwc_.push_back(bpu_model->inputs[0].shape.d[h_idx]);
  input_nhwc_.push_back(bpu_model->inputs[0].shape.d[w_idx]);
  input_nhwc_.push_back(bpu_model->inputs[0].shape.d[c_idx]);
  aligned_input_nhwc_.push_back(bpu_model->inputs[0].aligned_shape.d[0]);
  aligned_input_nhwc_.push_back(bpu_model->inputs[0].aligned_shape.d[h_idx]);
  aligned_input_nhwc_.push_back(bpu_model->inputs[0].aligned_shape.d[w_idx]);
  aligned_input_nhwc_.push_back(bpu_model->inputs[0].aligned_shape.d[c_idx]);
}
}  // namespace xstream
