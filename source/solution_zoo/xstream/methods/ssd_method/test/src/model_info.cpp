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

#include "ssd_method/model_info.h"
#include "bpu_predict/bpu_internal.h"
#include "hobotlog/hobotlog.hpp"
#include <string>
namespace xstream {

void ModelInfo::Init(BPUHandle bpuHandler,
                     std::string model_name,
                     BPUModelInfo *input_model_info,
                     BPUModelInfo *output_model_info) {
  model_name_ = model_name;
  //  model_file_path_ = model_file_path;
  aligned_nhwc_.resize(output_model_info->num);
  real_nhwc_.resize(output_model_info->num);
  output_layer_size_.resize(output_model_info->num);
  mxnet_output_layer_size_.resize(output_model_info->num);
  all_shift_.resize(output_model_info->num);

  // get info: aligned_nhwc_ and output_layer_size_
  for (int i = 0; i < output_model_info->num; ++i) {
    int model_out_size = 1;
    int mxnet_model_out_size = 1;
    for (int j = output_model_info->ndim_array[i];
         j < output_model_info->ndim_array[i + 1];
         ++j) {
      model_out_size *= output_model_info->aligned_shape_array[j];
      aligned_nhwc_[i].push_back(output_model_info->aligned_shape_array[j]);
      mxnet_model_out_size *= output_model_info->valid_shape_array[j];
      real_nhwc_[i].push_back(output_model_info->valid_shape_array[j]);
    }
    HOBOT_CHECK(real_nhwc_[i].size() == 4) << "real_nhwc_ size: "
                                           << real_nhwc_[i].size();

    for (unsigned int k = 0; k < real_nhwc_[i][3]; k++) {
      all_shift_[i].push_back(output_model_info->shift_value[i][k]);
    }

    int out_type_size = sizeof(int8_t);
    if (output_model_info->dtype_array[i] == BPU_DTYPE_FLOAT32) {
      out_type_size = sizeof(float);
    }
    elem_size_.push_back(out_type_size);
    output_layer_size_[i] = out_type_size * model_out_size;
    mxnet_output_layer_size_[i] = out_type_size * mxnet_model_out_size;
  }

  // get info: input_nhwc_
  for (int i = input_model_info->ndim_array[0];
       i < input_model_info->ndim_array[1];
       i++) {
    input_nhwc_.push_back(input_model_info->valid_shape_array[i]);
  }
}

std::ostream &operator<<(std::ostream &os, const ModelInfo &info) {
  os << "model name:" << info.model_name_ << std::endl;
  // output_layer_size
  os << "output layer size:" << std::endl;
  for (auto &o_size : info.output_layer_size_) {
    os << o_size << " ";
  }
  os << std::endl;
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
  os << "shift:" << std::endl;
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
  return os;
}

}  // namespace xstream
