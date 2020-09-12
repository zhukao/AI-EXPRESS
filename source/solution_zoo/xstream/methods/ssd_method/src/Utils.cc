// Copyright (c) 2020 Horizon Robotics.All Rights Reserved.
//
// The material in this file is confidential and contains trade secrets
// of Horizon Robotics Inc. This is proprietary information owned by
// Horizon Robotics Inc. No part of this work may be disclosed,
// reproduced, copied, transmitted, or used in any way for any purpose,
// without the express written permission of Horizon Robotics Inc.

#include "ssd_method/Utils.h"

#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>

#include "bpu_predict/bpu_predict_extension.h"
#include "hobotlog/hobotlog.hpp"

uint64_t GetCurrentTime() {
  auto current = std::chrono::system_clock::now();
  auto m = current.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(m).count();
}

int LoadModel(std::string &model_file, BPU_MODEL_S *bpu_model) {
  std::ifstream ifs(model_file.c_str(), std::ios::in | std::ios::binary);
  if (!ifs) {
    LOGE << "Open " << model_file << " failed";
    return -1;
  }
  ifs.seekg(0, std::ios::end);
  int model_length = ifs.tellg();
  ifs.seekg(0, std::ios::beg);
  char *model_bin = new char[sizeof(char) * model_length];
  ifs.read(model_bin, model_length);
  ifs.close();
  int ret_code = HB_BPU_loadModel(model_bin, model_length, bpu_model);
  delete[] model_bin;
  return ret_code;
}

void ReleaseImageInputTensors(std::vector<BPU_TENSOR_S> &tensors) {
  for (size_t i = 0; i < tensors.size(); i++) {
    BPU_TENSOR_S *tensor = &tensors[i];
    switch (tensor->data_type) {
      case BPU_TYPE_IMG_BGRP:
      case BPU_TYPE_IMG_RGBP:
      case BPU_TYPE_IMG_Y:
      case BPU_TYPE_IMG_RGB:
      case BPU_TYPE_IMG_BGR:
      case BPU_TYPE_IMG_YUV444:
      case BPU_TYPE_IMG_YUV_NV12:
        HB_SYS_bpuMemFree(&(tensor->data));
        break;
      case BPU_TYPE_IMG_NV12_SEPARATE:
        HB_SYS_bpuMemFree(&(tensor->data));
        HB_SYS_bpuMemFree(&(tensor->data_ext));
        break;
      default:
        break;
    }
  }
}

void PrepareOutputTensors(BPU_MODEL_S *bpu_model,
                          std::vector<BPU_TENSOR_S> &tensors,
                          const int32_t max_bbox_size) {
  HOBOT_CHECK(bpu_model) << "Null bpu_model";
  HOBOT_CHECK(max_bbox_size == 1);
  tensors.resize(bpu_model->output_num * max_bbox_size);

  for (int i = 0; i < max_bbox_size; i++) {
    for (int j = 0; j < bpu_model->output_num; j++) {
      BPU_TENSOR_S &tensor = tensors[i * bpu_model->output_num + j];
      BPU_MODEL_NODE_S &node = bpu_model->outputs[j];

      int output_size = 1;
      for (int k = 0; k < node.aligned_shape.ndim; k++) {
        output_size *= node.aligned_shape.d[k];
      }
      if (node.data_type == BPU_TYPE_TENSOR_F32 ||
          node.data_type == BPU_TYPE_TENSOR_S32 ||
          node.data_type == BPU_TYPE_TENSOR_U32) {
        output_size *= 4;
      }
      HB_SYS_bpuMemAlloc("out_data", output_size, true, &tensor.data);
    }
  }
}

void ReleaseBPUBuffer(std::vector<BPU_TENSOR_S> &tensors) {
  for (size_t i = 0; i < tensors.size(); i++) {
    HB_SYS_bpuMemFree(&tensors[i].data);
  }
}

void PrintModelInfo(BPU_MODEL_S *bpu_model) {
  auto shape_str_fn = [](BPU_DATA_SHAPE_S *shape) {
    std::stringstream ss;
    ss << "(";
    std::copy(shape->d, shape->d + shape->ndim,
              std::ostream_iterator<int>(ss, ","));
    ss << ")";
    ss << ", layout:" << shape->layout;
    return ss.str();
  };

  //  std::stringstream ss;
  LOGI << "Input num:" << bpu_model->input_num;
  for (int i = 0; i < bpu_model->input_num; i++) {
    auto &input_node = bpu_model->inputs[i];
    LOGI << ", input[" << i << "]: "
         << "name:" << input_node.name << ", data type:" << input_node.data_type
         << ", shape:" << shape_str_fn(&input_node.shape)
         << ", aligned shape:" << shape_str_fn(&input_node.aligned_shape);
  }

  LOGI << ", Output num:" << bpu_model->output_num;
  for (int i = 0; i < bpu_model->output_num; i++) {
    auto &output_node = bpu_model->outputs[i];
    LOGI << ", output[" << i << "]: "
         << "name:" << output_node.name << ", op:" << output_node.op_type
         << ", data type:" << output_node.data_type
         << ", shape:" << shape_str_fn(&output_node.shape)
         << ", aligned shape:" << shape_str_fn(&output_node.aligned_shape);
  }
}

void InitClsName(const std::string &cls_name_file,
                 std::vector<std::string> &cls_names) {
  std::ifstream fi(cls_name_file);
  if (fi) {
    std::string line;
    while (std::getline(fi, line)) {
      cls_names.push_back(line);
    }
  } else {
    LOGE << "can not open cls name file";
  }
}
#ifdef X3
void Image2BPUTensor(const PymImageFrame &info, int pyramid_layer,
                     BPU_TENSOR_S &tensor) {
  tensor.data_type = BPU_TYPE_IMG_NV12_SEPARATE;
  int h_idx, w_idx, c_idx;
  HB_BPU_getHWCIndex(tensor.data_type, nullptr, &h_idx, &w_idx, &c_idx);
  tensor.data_shape.ndim = 4;
  tensor.data_shape.d[0] = 1;
  tensor.data_shape.d[c_idx] = 3;
  tensor.data_shape.d[h_idx] = info.down_scale[pyramid_layer].height;
  tensor.data_shape.d[w_idx] = info.down_scale[pyramid_layer].width;
  uint8_t *data =
      reinterpret_cast<uint8_t *>(info.down_scale[pyramid_layer].y_vaddr);

  // Align by 16 bytes
  int stride = ALIGN_16(info.down_scale[pyramid_layer].width);
  int y_length = info.down_scale[pyramid_layer].height * stride;
  int uv_length = info.down_scale[pyramid_layer].height / 2 * stride;
  // Copy y data to data0
  HB_SYS_bpuMemAlloc("in_data", y_length + uv_length, true, &tensor.data);
  uint8_t *y = reinterpret_cast<uint8_t *>(tensor.data.virAddr);
  for (int h = 0; h < info.down_scale[pyramid_layer].height; ++h) {
    auto *raw = y + h * stride;
    memcpy(raw, data, info.down_scale[pyramid_layer].width);
    data += info.down_scale[pyramid_layer].width;
  }
  if (HB_SYS_isMemCachable(&tensor.data))
    HB_SYS_flushMemCache(&tensor.data, HB_SYS_MEM_CACHE_CLEAN);

  // Copy uv data to data_ext
  data = reinterpret_cast<uint8_t *>(info.down_scale[pyramid_layer].c_vaddr);
  HB_SYS_bpuMemAlloc("in_data1", uv_length, true, &tensor.data_ext);
  uint8_t *uv = reinterpret_cast<uint8_t *>(tensor.data_ext.virAddr);
  int uv_height = info.down_scale[pyramid_layer].height / 2;
  for (int i = 0; i < uv_height; ++i) {
    auto *raw = uv + i * stride;
    memcpy(raw, data, info.down_scale[pyramid_layer].width);
    data += info.down_scale[pyramid_layer].width;
  }
  if (HB_SYS_isMemCachable(&tensor.data_ext))
    HB_SYS_flushMemCache(&tensor.data_ext, HB_SYS_MEM_CACHE_CLEAN);

  tensor.aligned_shape.ndim = 4;
  tensor.aligned_shape.d[0] = 1;
  tensor.aligned_shape.d[h_idx] = info.down_scale[pyramid_layer].height;
  tensor.aligned_shape.d[w_idx] = stride;
  tensor.aligned_shape.d[c_idx] = 3;
}
#endif
#ifdef X2
void Image2BPUTensor(const img_info_t &info, int pyramid_layer,
                     BPU_TENSOR_S &tensor) {
  tensor.data_type = BPU_TYPE_IMG_NV12_SEPARATE;
  int h_idx, w_idx, c_idx;
  HB_BPU_getHWCIndex(tensor.data_type, nullptr, &h_idx, &w_idx, &c_idx);
  tensor.data_shape.ndim = 4;
  tensor.data_shape.d[0] = 1;
  tensor.data_shape.d[c_idx] = 3;
  tensor.data_shape.d[h_idx] = info.down_scale[pyramid_layer].height;
  tensor.data_shape.d[w_idx] = info.down_scale[pyramid_layer].width;
  uint8_t *data =
      reinterpret_cast<uint8_t *>(info.down_scale[pyramid_layer].y_vaddr);

  // Align by 16 bytes
  int stride = ALIGN_16(info.down_scale[pyramid_layer].width);
  int y_length = info.down_scale[pyramid_layer].height * stride;
  int uv_length = info.down_scale[pyramid_layer].height / 2 * stride;
  // Copy y data to data0
  HB_SYS_bpuMemAlloc("in_data", y_length, true, &tensor.data);
  uint8_t *y = reinterpret_cast<uint8_t *>(tensor.data.virAddr);
  for (int h = 0; h < info.down_scale[pyramid_layer].height; ++h) {
    auto *raw = y + h * stride;
    memcpy(raw, data, info.down_scale[pyramid_layer].width);
    data += info.down_scale[pyramid_layer].width;
  }
  if (HB_SYS_isMemCachable(&tensor.data))
    HB_SYS_flushMemCache(&tensor.data, HB_SYS_MEM_CACHE_CLEAN);
  // Copy uv data to data1
  data = reinterpret_cast<uint8_t *>(info.down_scale[pyramid_layer].c_vaddr);
  HB_SYS_bpuMemAlloc("in_data1", uv_length, true, &tensor.data_ext);
  uint8_t *uv = reinterpret_cast<uint8_t *>(tensor.data_ext.virAddr);
  int uv_height = info.down_scale[pyramid_layer].height / 2;
  for (int i = 0; i < uv_height; ++i) {
    auto *raw = uv + i * stride;
    memcpy(raw, data, info.down_scale[pyramid_layer].width);
    data += info.down_scale[pyramid_layer].width;
  }
  tensor.aligned_shape.ndim = 4;
  tensor.aligned_shape.d[0] = 1;
  tensor.aligned_shape.d[h_idx] = info.down_scale[pyramid_layer].height;
  tensor.aligned_shape.d[w_idx] = stride;
  tensor.aligned_shape.d[c_idx] = 3;
  if (HB_SYS_isMemCachable(&tensor.data_ext))
    HB_SYS_flushMemCache(&tensor.data_ext, HB_SYS_MEM_CACHE_CLEAN);
}
#endif

