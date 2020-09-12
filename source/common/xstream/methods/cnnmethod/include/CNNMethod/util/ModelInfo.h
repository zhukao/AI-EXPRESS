/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: ModelInfo.h
 * @Brief: declaration of the ModelInfo
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-05-12 14:18:28
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-05-12 16:16:58
 */

#ifndef INCLUDE_CNNMETHOD_UTIL_MODELINFO_H_
#define INCLUDE_CNNMETHOD_UTIL_MODELINFO_H_

#include <iostream>
#include <string>
#include <vector>
#include "bpu_predict/bpu_io.h"
#include "bpu_predict/bpu_predict.h"
#include "bpu_predict/bpu_predict_extension.h"
namespace xstream {

class ModelInfo {
 public:
  void Init(BPU_MODEL_S *bpu_model, int data_type);
  friend std::ostream &operator<<(std::ostream &o,
                                  const ModelInfo &info);

 public:
  // output info
  std::vector<int> output_layer_size_;               // include sizeof(ele)
  std::vector<int> mxnet_output_layer_size_;         // include sizeof(ele)
  std::vector<std::vector<uint32_t>> aligned_nhwc_;  // bpu output nhwc
  std::vector<std::vector<uint32_t>> real_nhwc_;     // mxnet output nhwc
  std::vector<uint32_t> elem_size_;
  std::vector<std::vector<uint32_t>> all_shift_;
  // input info
  std::vector<int> input_nhwc_;
  std::vector<int> aligned_input_nhwc_;

  // data_type 定点模型需要指定输入类型 输出类型默认float
  BPU_DATA_TYPE_E data_type_ = BPU_TYPE_IMG_NV12_SEPARATE;
};

}  // namespace xstream
#endif  // INCLUDE_CNNMETHOD_UTIL_MODELINFO_H_
