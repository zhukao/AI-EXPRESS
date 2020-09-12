/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: PostProcessor.cc
 * @Brief: definition of the PostProcessor
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-28 00:28:17
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-28 00:32:08
 */

#include "PostProcessMethod/PostProcessor/PostProcessor.h"
#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include<string.h>
#include "json/json.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

int PostProcessor::Init(const std::string &cfg) {
  LOGD << "PostProcessor Init";

  // string to json
  Json::Value config;
  {
    Json::CharReaderBuilder readerBuilder;
    JSONCPP_STRING errs;
    std::unique_ptr<Json::CharReader> jsonReader(readerBuilder.newCharReader());
    jsonReader->parse(
        cfg.c_str(), cfg.c_str() + cfg.length(), &config, &errs);
  }
  config_.reset(new Config(config));
  return 0;
}

void PostProcessor::Finalize() {
  input_tensors_ = nullptr;
  output_tensors_ = nullptr;
  task_handle_ = nullptr;
  bpu_model_ = nullptr;
}

int PostProcessor::ReleaseTensor(
    std::shared_ptr<std::vector<BPU_TENSOR_S>> tensors) {
  for (size_t i = 0; i < tensors->size(); i++) {
    BPU_TENSOR_S &tensor = tensors->at(i);
    switch (tensor.data_type) {
      case BPU_TYPE_IMG_Y:
      case BPU_TYPE_IMG_YUV_NV12:
      case BPU_TYPE_IMG_YUV444:
      case BPU_TYPE_IMG_RGB:
      case BPU_TYPE_IMG_BGR:
      case BPU_TYPE_IMG_BGRP:
      case BPU_TYPE_IMG_RGBP:
      case BPU_TYPE_TENSOR_U8:
      case BPU_TYPE_TENSOR_S8:
      case BPU_TYPE_TENSOR_F32:
      case BPU_TYPE_TENSOR_S32:
      case BPU_TYPE_TENSOR_U32:
        HB_SYS_bpuMemFree(&tensor.data);
        break;
      case BPU_TYPE_IMG_NV12_SEPARATE:
        HB_SYS_bpuMemFree(&tensor.data);
        HB_SYS_bpuMemFree(&tensor.data_ext);
        break;
      default:
        HOBOT_CHECK(0) << "not support data_type: " << tensor.data_type;
        break;
    }
  }
  return 0;
}

}  // namespace xstream
