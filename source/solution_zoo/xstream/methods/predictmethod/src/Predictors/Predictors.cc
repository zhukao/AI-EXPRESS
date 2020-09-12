/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: Predictors.cc
 * @Brief: definition of the Predictors
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-22 14:28:17
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-22 16:19:08
 */

#include "PredictMethod/Predictors/Predictors.h"
#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include<string.h>
#include "json/json.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

std::mutex Predictors::init_mutex_;
int Predictors::Init(const std::string &cfg) {
  LOGD << "Predictors Init";
  // 0. parse config_
  {
    Json::CharReaderBuilder readerBuilder;
    JSONCPP_STRING errs;
    std::unique_ptr<Json::CharReader> jsonReader(readerBuilder.newCharReader());
    jsonReader->parse(
        cfg.c_str(), cfg.c_str() + cfg.length(), &config_, &errs);
  }

  // 1. model_path_
  model_path_ = config_["model_file_path"].isString() ?
                config_["model_file_path"].asString() : "";
  HOBOT_CHECK(model_path_.size() > 0) << "must set model_file_path";

  // 2. load model
  std::unique_lock<std::mutex> lock(init_mutex_);
  int ret = 0;
  bpu_model_ = std::make_shared<BPU_MODEL_S>();
  ret = HB_BPU_loadModelFromFile(model_path_.c_str(), bpu_model_.get());
  HOBOT_CHECK(ret == 0) << "load model failed: " << HB_BPU_getErrorName(ret);

  // 3. 获取模型输入大小
  HOBOT_CHECK(bpu_model_->input_num == 1);
  int height, width;
  ret = HB_BPU_getHW(bpu_model_->inputs[0].data_type,
                     &bpu_model_->inputs[0].shape,
                     &height, &width);
  HOBOT_CHECK(ret == 0) << "load model input size failed: "
      << HB_BPU_getErrorName(ret);
  model_input_height_ = height;
  model_input_width_ = width;

  // 4. 获取模型输入hwc索引
  ret = HB_BPU_getHWCIndex(bpu_model_->inputs[0].data_type,
                           &bpu_model_->inputs[0].shape.layout,
                           &input_h_idx_, &input_w_idx_, &input_c_idx_);
  HOBOT_CHECK(ret == 0) << "load model input index failed: "
      << HB_BPU_getErrorName(ret);

  // 5. 获取金字塔层数
  pyramid_layer_ = config_["pyramid_layer"].isInt() ?
                   config_["pyramid_layer"].asInt() : pyramid_layer_;
  return 0;
}

void Predictors::Finalize() {
  if (bpu_model_) {
    HB_BPU_releaseModel(bpu_model_.get());
  }
}

int Predictors::PrepareInputTensorFromPym(
    uint8_t* y_data, uint8_t* uv_data, int y_len, int uv_len,
    std::shared_ptr<std::vector<BPU_TENSOR_S>> input_tensors,
    BPU_DATA_TYPE_E data_type) {
  // 1. prepare input
  input_tensors->resize(bpu_model_->input_num);
  for (int i = 0; i < bpu_model_->input_num; i++) {
    BPU_TENSOR_S &tensor = input_tensors->at(i);
    BPU_MODEL_NODE_S &node = bpu_model_->inputs[i];
    tensor.data_type = data_type;
    tensor.data_shape.layout = node.shape.layout;
    tensor.aligned_shape.layout = node.shape.layout;

    LOGD << "node data_type: " << node.data_type;

    tensor.data_shape.ndim = 4;
    tensor.data_shape.d[0] = 1;
    tensor.data_shape.d[input_h_idx_] = node.shape.d[input_h_idx_];
    tensor.data_shape.d[input_w_idx_] = node.shape.d[input_w_idx_];
    tensor.data_shape.d[input_c_idx_] = node.shape.d[input_c_idx_];
    tensor.aligned_shape.ndim = 4;
    tensor.aligned_shape.d[0] = 1;
    tensor.aligned_shape.d[input_h_idx_] = node.aligned_shape.d[input_h_idx_];
    tensor.aligned_shape.d[input_w_idx_] = node.aligned_shape.d[input_w_idx_];
    tensor.aligned_shape.d[input_c_idx_] = node.aligned_shape.d[input_c_idx_];
    LOGD << "input_tensor.data_shape.d[0]: " << tensor.data_shape.d[0] << ", "
         << "input_tensor.data_shape.d[1]: " << tensor.data_shape.d[1] << ", "
         << "input_tensor.data_shape.d[2]: " << tensor.data_shape.d[2] << ", "
         << "input_tensor.data_shape.d[3]: " << tensor.data_shape.d[3] << ", "
         << "input_tensor.data_shape.layout: " << tensor.data_shape.layout;

    int image_height = tensor.data_shape.d[input_h_idx_];
    int image_width = tensor.data_shape.d[input_w_idx_];
    int image_channel = tensor.data_shape.d[input_c_idx_];
    int stride = tensor.aligned_shape.d[input_w_idx_];
    LOGD << "image_height: " << image_height << ", "
         << "image_width: " << image_width << ", "
         << "image channel: " << image_channel << ", "
         << "stride: " << stride;

    switch (data_type) {
      case BPU_TYPE_IMG_NV12_SEPARATE: {
        int y_length = image_height * stride;
        int uv_length = image_height / 2 * stride;
        HOBOT_CHECK(y_length <= y_len && uv_length <= uv_len);
        HB_SYS_bpuMemAlloc("in_data0", y_length, true, &tensor.data);
        HB_SYS_bpuMemAlloc("in_data1", uv_length, true, &tensor.data_ext);
        // Copy y data to data0
        uint8_t *y = reinterpret_cast<uint8_t *>(tensor.data.virAddr);
        for (int h = 0; h < image_height; ++h) {
          auto *raw = y + h * stride;
          memcpy(raw, y_data, image_width);
          y_data += image_width;
        }
        HB_SYS_flushMemCache(&tensor.data, HB_SYS_MEM_CACHE_CLEAN);

        // Copy uv data to data_ext
        uint8_t *uv = reinterpret_cast<uint8_t *>(tensor.data_ext.virAddr);
        int uv_height = image_height / 2;
        for (int i = 0; i < uv_height; ++i) {
          auto *raw = uv + i * stride;
          memcpy(raw, uv_data, image_width);
          uv_data += image_width;
        }
        HB_SYS_flushMemCache(&tensor.data_ext, HB_SYS_MEM_CACHE_CLEAN);
        break;
      }
      default:
        HOBOT_CHECK(0) << "unsupport data_type: " << data_type;
        break;
    }
  }
  return 0;
}

int Predictors::PrepareInputTensorData(
    uint8_t *img_data, int data_length,
    std::shared_ptr<std::vector<BPU_TENSOR_S>> input_tensors) {
  input_tensors->resize(bpu_model_->input_num);
  for (int i = 0; i < bpu_model_->input_num; i++) {
    BPU_TENSOR_S &tensor = input_tensors->at(i);
    BPU_MODEL_NODE_S &node = bpu_model_->inputs[i];
    tensor.data_type = node.data_type;
    if (tensor.data_type == BPU_TYPE_IMG_YUV_NV12) {
      tensor.data_type = BPU_TYPE_IMG_NV12_SEPARATE;
    }
    tensor.data_shape.layout = node.shape.layout;
    tensor.aligned_shape.layout = node.shape.layout;

    tensor.data_shape.ndim = 4;
    tensor.data_shape.d[0] = 1;
    tensor.data_shape.d[input_h_idx_] = node.shape.d[input_h_idx_];
    tensor.data_shape.d[input_w_idx_] = node.shape.d[input_w_idx_];
    tensor.data_shape.d[input_c_idx_] = node.shape.d[input_c_idx_];
    tensor.aligned_shape.ndim = 4;
    tensor.aligned_shape.d[0] = 1;
    tensor.aligned_shape.d[input_h_idx_] = node.aligned_shape.d[input_h_idx_];
    tensor.aligned_shape.d[input_w_idx_] = node.aligned_shape.d[input_w_idx_];
    tensor.aligned_shape.d[input_c_idx_] = node.aligned_shape.d[input_c_idx_];
    LOGD << "input_tensor.data_shape.d[0]: " << tensor.data_shape.d[0] << ", "
         << "input_tensor.data_shape.d[1]: " << tensor.data_shape.d[1] << ", "
         << "input_tensor.data_shape.d[2]: " << tensor.data_shape.d[2] << ", "
         << "input_tensor.data_shape.d[3]: " << tensor.data_shape.d[3] << ", "
         << "input_tensor.data_shape.layout: " << tensor.data_shape.layout;

    int image_height = tensor.data_shape.d[input_h_idx_];
    int image_width = tensor.data_shape.d[input_w_idx_];
    int image_channel = tensor.data_shape.d[input_c_idx_];
    LOGD << "image_height: " << image_height << ", "
         << "image_width: " << image_width << ", "
         << "image channel: " << image_channel;

    switch (tensor.data_type) {
      case BPU_TYPE_IMG_NV12_SEPARATE: {
        int stride = tensor.aligned_shape.d[input_w_idx_];
        int y_length = image_height * stride;
        int uv_length = image_height / 2 * stride;
        HB_SYS_bpuMemAlloc("in_data0", y_length, true, &tensor.data);
        HB_SYS_bpuMemAlloc("in_data1", uv_length, true, &tensor.data_ext);
        HOBOT_CHECK(image_height*image_width*3/2 == data_length)
            << "Input img length error!";
        // Copy y data to data0
        uint8_t *y = reinterpret_cast<uint8_t *>(tensor.data.virAddr);
        for (int h = 0; h < image_height; ++h) {
          auto *raw = y + h * stride;
          memcpy(raw, img_data, image_width);
          img_data += image_width;
        }
        HB_SYS_flushMemCache(&tensor.data, HB_SYS_MEM_CACHE_CLEAN);

        // Copy uv data to data_ext
        uint8_t *uv = reinterpret_cast<uint8_t *>(tensor.data_ext.virAddr);
        int uv_height = image_height / 2;
        for (int i = 0; i < uv_height; ++i) {
          auto *raw = uv + i * stride;
          memcpy(raw, img_data, image_width);
          img_data += image_width;
        }
        HB_SYS_flushMemCache(&tensor.data_ext, HB_SYS_MEM_CACHE_CLEAN);
        break;
      }
      default:
        HOBOT_CHECK(0) << "unsupport data_type: " << tensor.data_type;
        break;
    }
  }
  return 0;
}

int Predictors::ResizeInputTensor(
    std::shared_ptr<std::vector<BPU_TENSOR_S>> pre_tensors,
    std::shared_ptr<std::vector<BPU_TENSOR_S>> tensors) {
  HOBOT_CHECK(pre_tensors->size() == tensors->size() && tensors->size() == 1);
  BPU_RESIZE_CTRL_S ctrl_param;
  ctrl_param.resize_type = BPU_RESIZE_TYPE_BILINEAR;
  ctrl_param.output_type = tensors->at(0).data_type;

  int ret = HB_BPU_resize(pre_tensors->data(), tensors->data(), &ctrl_param);
  if (ret != 0) {
    LOGE << "bpu resize input tensor failed: " << HB_BPU_getErrorName(ret);
    return ret;
  }
  return 0;
}

int Predictors::PrepareOutputTensor(
    std::shared_ptr<std::vector<BPU_TENSOR_S>> tensors,
    int batch_size) {
  tensors->resize(bpu_model_->output_num);
  for (int i = 0; i < bpu_model_->output_num; i++) {
    BPU_TENSOR_S &tensor = tensors->at(i);
    BPU_MODEL_NODE_S &node = bpu_model_->outputs[i];
    tensor.data_type = node.data_type;
    tensor.data_shape = node.shape;
    tensor.aligned_shape = node.aligned_shape;
    int output_size = 1;
    for (int j = 0; j < node.aligned_shape.ndim; j++) {
      output_size *= node.aligned_shape.d[j];
      LOGD << "node.aligned_shape.d[j]:" << node.aligned_shape.d[j];
    }
    if (node.data_type == BPU_TYPE_TENSOR_F32 ||
        node.data_type == BPU_TYPE_TENSOR_S32 ||
        node.data_type == BPU_TYPE_TENSOR_U32) {
      output_size *= 4;
    }
    LOGD << "output_size: " << output_size;
    LOGD << "batch_size: " << batch_size;
    HB_SYS_bpuMemAlloc("out_data0",
                       output_size * batch_size,
                       true, &tensor.data);
  }
  return 0;
}

int Predictors::ReleaseTensor(
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
