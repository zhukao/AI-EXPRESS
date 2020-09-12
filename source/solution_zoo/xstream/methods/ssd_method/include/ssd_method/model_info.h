/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: model_info.h
 * @Brief: declaration of the ModelInfo
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-05-12 14:18:28
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-05-12 16:16:58
 */

#ifndef INCLUDE_SSDMETHOD_MODELINFO_H_
#define INCLUDE_SSDMETHOD_MODELINFO_H_

#include <iostream>
#include <string>
#include <vector>
#include "bpu_predict/bpu_io.h"
#include "bpu_predict/bpu_predict.h"
namespace xstream {

class ModelInfo {
 public:
  void Init(BPUHandle bpuHandler, std::string model_name,
            BPUModelInfo *input_model, BPUModelInfo *output_model);
  friend std::ostream &operator<<(std::ostream &o, const ModelInfo &info);

 public:
  std::string model_name_;
  std::string model_file_path_;

  // output info
  std::vector<int> output_layer_size_;               // include sizeof(ele)
  std::vector<int> mxnet_output_layer_size_;         // include sizeof(ele)
  std::vector<std::vector<uint32_t>> aligned_nhwc_;  // bpu output nhwc
  std::vector<std::vector<uint32_t>> real_nhwc_;     // mxnet output nhwc
  std::vector<uint32_t> elem_size_;
  std::vector<std::vector<uint32_t>> all_shift_;
  // input info
  std::vector<int> input_nhwc_;
};

}  // namespace xstream
#endif  // INCLUDE_SSDMETHOD_MODELINFO_H_
