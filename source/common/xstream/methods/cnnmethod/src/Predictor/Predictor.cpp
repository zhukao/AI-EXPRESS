/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: Predictor.cpp
 * @Brief: definition of the Predictor
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-07-16 14:27:05
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-07-16 16:23:27
 */

#include "CNNMethod/Predictor/Predictor.h"
#include <stdint.h>
#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "CNNMethod/util/util.h"
#include "hb_vio_interface.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"
#include "horizon/vision_type/vision_type.hpp"
#include "horizon/vision_type/vision_type_common.h"
#ifdef X3
#include "./bpu_predict_x3.h"
#endif

namespace xstream {
int32_t Predictor::Init(std::shared_ptr<CNNMethodConfig> config) {
  model_name_ = config->GetSTDStringValue("model_name");
  model_version_ = config->GetSTDStringValue("model_version", "unknown");
  HOBOT_CHECK(model_name_.size() > 0) << "must set model_name";

  post_fn_ = config->GetSTDStringValue("post_fn");

  std::string parent_path = config->GetSTDStringValue("parent_path");
  model_path_ = config->GetSTDStringValue("model_file_path");
  std::string bpu_cfg_path = config->GetSTDStringValue("bpu_config_path");
  max_handle_num_ = config->GetIntValue("max_handle_num", -1);
  HOBOT_CHECK(model_path_.size() > 0) << "must set model_file_path";
  HOBOT_CHECK(bpu_cfg_path.size() > 0) << "must set bpu_config_cfg";

  model_path_ = parent_path + model_path_;
  bpu_cfg_path = parent_path + bpu_cfg_path;
  LOGD << "model_file_path:" << model_path_ << std::endl
       << "bpu_config_path:" << bpu_cfg_path << std::endl
       << "parent path" << parent_path;
  core_id_ = config->GetIntValue("core_id", 2);

  int ret = 0;
  // new bpu
  // load model
  {
    std::ifstream ifs(model_path_.c_str(), std::ios::in | std::ios::binary);
    if (!ifs) {
      HOBOT_CHECK(0) << "Open model file: " << model_path_ << " failed";
    }
    ifs.seekg(0, std::ios::end);
    int model_length = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    char *model_bin = new char[sizeof(char) * model_length];
    ifs.read(model_bin, model_length);
    ifs.close();
    bpu_model_ = new BPU_MODEL_S();
    ret = HB_BPU_loadModel(model_bin, model_length, bpu_model_);
    HOBOT_CHECK(ret == 0) << "load model failed" << HB_BPU_getErrorName(ret);
    delete[] model_bin;
  }
  // data_type_
  {
    std::string data_type = config->GetSTDStringValue("data_type");
    if (data_type != "") {
      auto iter = g_data_type_map.find(data_type);
      HOBOT_CHECK(iter != g_data_type_map.end())
          << "data_type unknown: " << data_type;
      data_type_ = iter->second;
    }
  }
  // model_info_
  model_info_.Init(bpu_model_, data_type_);

  return 0;
}

void Predictor::UpdateParam(std::shared_ptr<CNNMethodConfig> config) {
  if (config->KeyExist("max_handle_num")) {
    max_handle_num_ = config->GetIntValue("max_handle_num");
  }
}

void Predictor::Finalize() {
  if (output_tensors_alloced_) {
    ReleaseOutputTensor();
    output_tensors_alloced_ = false;
  }
  if (input_tensors_alloced_) {
    ReleaseInputTensor();
    input_tensors_alloced_ = false;
  }
  if (bpu_model_) {
    // release model
    HB_BPU_releaseModel(bpu_model_);
  }
}

// only support resizer input model
int Predictor::RunModelWithBBox(
    hobot::vision::PymImageFrame &pym_image,
    BPU_BBOX *box, int box_num, int *resizable_cnt) {
  PrepareOutputTensorBatch(box_num);

  BPU_TASK_HANDLE task_handle;
  BPU_RUN_CTRL_S run_ctrl;
  run_ctrl.core_id = core_id_;
#ifdef X2
  int ret = HB_BPU_runModelWithBbox(
      bpu_model_,
      static_cast<BPU_CAMERA_BUFFER>(&pym_image.img),
      box, box_num, output_tensors_.data(),
      bpu_model_->output_num, &run_ctrl,
      true, resizable_cnt, &task_handle);
#endif

#ifdef X3
  bpu_predict_x3::PyramidResult bpu_predict_pyramid;
  Convert(pym_image, bpu_predict_pyramid);

  int ret = HB_BPU_runModelWithBbox(
      bpu_model_,
      static_cast<BPU_CAMERA_BUFFER>(&bpu_predict_pyramid.result_info),
      box, box_num, output_tensors_.data(),
      bpu_model_->output_num, &run_ctrl,
      true, resizable_cnt, &task_handle);

#endif
  if (ret != 0 && *resizable_cnt == 0) {
    LOGI << "no box pass resizer";
    // ReleaseOutputTensor();
    return -1;
  } else if (ret != 0) {
    LOGE << "RunModelWithBBox failed, " << HB_BPU_getErrorName(ret);
    // ReleaseOutputTensor();
    return -1;
  }

  LOGD << "resizeable box:" << *resizable_cnt;
  return 0;
}

int Predictor::NormalizeRoi(hobot::vision::BBox *src,
                            hobot::vision::BBox *dst,
                            NormParams norm_params,
                            uint32_t total_w,
                            uint32_t total_h,
                            FilterMethod filter_method) {
  *dst = *src;
  float box_w = dst->x2 - dst->x1;
  float box_h = dst->y2 - dst->y1;
  float center_x = (dst->x1 + dst->x2) / 2.0f;
  float center_y = (dst->y1 + dst->y2) / 2.0f;
  float w_new = box_w;
  float h_new = box_h;
  NormMethod norm_method = norm_params.norm_type;
  float norm_ratio = norm_params.expand_scale;
  float aspect_ratio = norm_params.aspect_ratio;

  switch (norm_method) {
    case NormMethod::BPU_MODEL_NORM_BY_WIDTH_LENGTH: {
      w_new = box_w * norm_ratio;
      h_new = box_h + w_new - box_w;
      if (h_new <= 0) return -1;
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_WIDTH_RATIO:
    case NormMethod::BPU_MODEL_NORM_BY_HEIGHT_RATIO:
    case NormMethod::BPU_MODEL_NORM_BY_LSIDE_RATIO: {
      h_new = box_h * norm_ratio;
      w_new = box_w * norm_ratio;
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_HEIGHT_LENGTH: {
      h_new = box_h * norm_ratio;
      w_new = box_w + h_new - box_h;
      if (w_new <= 0) return -1;
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_LSIDE_LENGTH: {
      if (box_w > box_h) {
        w_new = box_w * norm_ratio;
        h_new = box_h + w_new - box_w;
        if (h_new <= 0) return -1;
      } else {
        h_new = box_h * norm_ratio;
        w_new = box_w + h_new - box_h;
        if (w_new <= 0) return -1;
      }
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_LSIDE_SQUARE: {
      if (box_w > box_h) {
        w_new = box_w * norm_ratio;
        h_new = w_new / aspect_ratio;
      } else {
        h_new = box_h * norm_ratio;
        w_new = h_new * aspect_ratio;
      }
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_DIAGONAL_SQUARE: {
      float diagonal = sqrt(pow(box_w, 2.0) + pow(box_h, 2.0));
      w_new = h_new = diagonal * norm_ratio;
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_WIDTH_SQUARE: {
        w_new = box_w * norm_ratio;
        h_new = w_new;
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_HEIGHT_SQUARE: {
        h_new = box_h * norm_ratio;
        w_new = h_new;
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_NOTHING:
      break;
    default:
      return 0;
  }
  dst->x1 = center_x - w_new / 2;
  dst->x2 = center_x + w_new / 2;
  dst->y1 = center_y - h_new / 2;
  dst->y2 = center_y + h_new / 2;

  if (FilterRoi(src, dst, total_w, total_h, filter_method)) {
    *dst = *src;
    return -1;
  }

  dst->x1 = dst->x1 < 0 ? 0.0f : dst->x1;
  dst->y1 = dst->y1 < 0 ? 0.0f : dst->y1;
  dst->x2 = dst->x2 > total_w ? total_w : dst->x2;
  dst->y2 = dst->y2 > total_h ? total_h : dst->y2;
  LOGD << "norm roi[x1, y1, x2, y2]: [" << dst->x1 << ", " << dst->y1 << ", "
       << dst->x2 << ", " << dst->y2 << "]";
  return 0;
}

int Predictor::FilterRoi(hobot::vision::BBox *src, hobot::vision::BBox *dst,
                         int src_w, int src_h, FilterMethod filter_method) {
  switch (filter_method) {
    case FilterMethod::OUT_OF_RANGE: {
      if (dst->x1 < 0 || dst->y1 < 0 || dst->x2 > src_w || dst->y2 > src_h)
        return -1;
    }
    case FilterMethod::NO_FILTER: {
        return 0;
    }
  }
  return 0;
}


void Predictor::PrepareInputTensor(uint8_t *img_data,
                                   int data_length,
                                   BPU_DATA_TYPE_E data_type,
                                   bool is_padding) {
  input_tensors_.resize(bpu_model_->input_num);
  for (int i = 0; i < bpu_model_->input_num; i++) {
    BPU_TENSOR_S &tensor = input_tensors_[i];
    BPU_MODEL_NODE_S &node = bpu_model_->inputs[i];
    tensor.data_type = data_type;
    tensor.data_shape.layout = node.shape.layout;
    tensor.aligned_shape.layout = node.shape.layout;

    int h_idx, w_idx, c_idx;
    HB_BPU_getHWCIndex(tensor.data_type,
                       &tensor.data_shape.layout,
                       &h_idx, &w_idx, &c_idx);
    int node_h_idx, node_w_idx, node_c_idx;
    HB_BPU_getHWCIndex(node.data_type,
                       &node.shape.layout,
                       &node_h_idx, &node_w_idx, &node_c_idx);
    tensor.data_shape.ndim = 4;
    tensor.data_shape.d[0] = 1;
    tensor.data_shape.d[h_idx] = node.shape.d[node_h_idx];
    tensor.data_shape.d[w_idx] = node.shape.d[node_w_idx];
    tensor.data_shape.d[c_idx] = node.shape.d[node_c_idx];
    tensor.aligned_shape.ndim = 4;
    tensor.aligned_shape.d[0] = 1;
    tensor.aligned_shape.d[h_idx] = node.aligned_shape.d[node_h_idx];
    tensor.aligned_shape.d[w_idx] = node.aligned_shape.d[node_w_idx];
    tensor.aligned_shape.d[c_idx] = node.aligned_shape.d[node_c_idx];
    LOGD << "input_tensor.data_shape.d[0]: " << tensor.data_shape.d[0] << ", "
         << "input_tensor.data_shape.d[1]: " << tensor.data_shape.d[1] << ", "
         << "input_tensor.data_shape.d[2]: " << tensor.data_shape.d[2] << ", "
         << "input_tensor.data_shape.d[3]: " << tensor.data_shape.d[3] << ", "
         << "input_tensor.data_shape.layout: " << tensor.data_shape.layout;

    int image_height = tensor.data_shape.d[h_idx];
    int image_width = tensor.data_shape.d[w_idx];
    int image_channel = tensor.data_shape.d[c_idx];
    LOGD << "image_height: " << image_height << ", "
         << "image_width: " << image_width << ", "
         << "image channel: " << image_channel;

    switch (data_type) {
      case BPU_TYPE_IMG_Y: {
        int stride = tensor.aligned_shape.d[w_idx];
        int image_length = image_height * stride;
        if (!input_tensors_alloced_) {
          HB_SYS_bpuMemAlloc("in_data0", image_length, true, &tensor.data);
        }
        uint8_t *data0 = reinterpret_cast<uint8_t *>(tensor.data.virAddr);
        HOBOT_CHECK(image_height*image_width == data_length)
            << "Input image data_length error!";

        for (int h = 0; h < image_height; ++h) {
          auto *raw = data0 + h * stride;
          memcpy(raw, img_data, image_width);
          img_data += image_width;
        }
        tensor.data_shape.d[c_idx] = 1;
        tensor.aligned_shape.d[c_idx] = 1;
        HB_SYS_flushMemCache(&tensor.data, HB_SYS_MEM_CACHE_CLEAN);
        break;
      }
      case BPU_TYPE_IMG_YUV444:
      case BPU_TYPE_IMG_BGR:
      case BPU_TYPE_IMG_RGB:
      case BPU_TYPE_IMG_BGRP:
      case BPU_TYPE_IMG_RGBP: {
        int image_length = image_height * image_width * 3;
        if (!input_tensors_alloced_) {
          HB_SYS_bpuMemAlloc("in_data0", image_length, true, &tensor.data);
        }
        HOBOT_CHECK(image_length == data_length)
            << "Input image data_length error!";

        memcpy(tensor.data.virAddr, img_data, image_length);
        HB_SYS_flushMemCache(&tensor.data, HB_SYS_MEM_CACHE_CLEAN);
        break;
      }
      case BPU_TYPE_IMG_YUV_NV12: {
        int stride = tensor.aligned_shape.d[w_idx];
        int y_length = image_height * stride;
        int uv_length = image_height / 2 * stride;
        if (!input_tensors_alloced_) {
          HB_SYS_bpuMemAlloc("in_data0", y_length + uv_length, true,
                             &tensor.data);
        }
        HOBOT_CHECK(image_height*image_width*3/2 == data_length)
            << "Input image data_length error!";

        // Copy y data to data0
        uint8_t *y = reinterpret_cast<uint8_t *>(tensor.data.virAddr);
        for (int h = 0; h < image_height; ++h) {
          auto *raw = y + h * stride;
          memcpy(raw, img_data, image_width);
          img_data += image_width;
        }
        // Copy uv data to data_ext
        uint8_t *uv = y + y_length;
        int uv_height = image_height / 2;
        for (int i = 0; i < uv_height; ++i) {
          auto *raw = uv + i * stride;
          memcpy(raw, img_data, image_width);
          img_data += image_width;
        }
        HB_SYS_flushMemCache(&tensor.data, HB_SYS_MEM_CACHE_CLEAN);
        break;
      }
      case BPU_TYPE_IMG_NV12_SEPARATE: {
        int stride = tensor.aligned_shape.d[w_idx];
        int y_length = image_height * stride;
        int uv_length = image_height / 2 * stride;
        if (!input_tensors_alloced_) {
          HB_SYS_bpuMemAlloc("in_data0", y_length, true, &tensor.data);
          HB_SYS_bpuMemAlloc("in_data1", uv_length, true, &tensor.data_ext);
        }
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
      case BPU_TYPE_TENSOR_S8: {
        int width_stride = tensor.aligned_shape.d[w_idx];
        int channel_stride = tensor.aligned_shape.d[c_idx];
        int length = image_height * width_stride * channel_stride;

        if (!input_tensors_alloced_) {
          HB_SYS_bpuMemAlloc("in_data0", length, true, &tensor.data);
        }
        LOGD << "length: " << length;
        LOGD << "data_length: " << data_length;
        if (!is_padding) {
          HOBOT_CHECK(image_height*image_width*image_channel == data_length)
              << "Input img length error!";
          // copy ddr data
          uint8_t *tensor_data = reinterpret_cast<uint8_t *>(
                                     tensor.data.virAddr);

          uint32_t dst_h_stride = width_stride * channel_stride;
          uint32_t dst_w_stride = channel_stride;
          uint32_t src_h_stride = image_width * image_channel;
          uint32_t src_w_stride = image_channel;
          for (int hh = 0; hh < image_height; ++hh) {
            uint8_t *cur_h_dst = tensor_data + hh * dst_h_stride;
            uint8_t *cur_h_src = img_data + hh * src_h_stride;
            for (int ww = 0; ww < width_stride; ++ww) {
              uint8_t *cur_w_dst = cur_h_dst + ww * dst_w_stride;
              uint8_t *cur_w_src = cur_h_src + ww * src_w_stride;
              memcpy(cur_w_dst, cur_w_src, image_channel);
            }
          }
        } else {
          HOBOT_CHECK(length == data_length)
              << "Input img length error!";
          // copy ddr data
          uint8_t *tensor_data = reinterpret_cast<uint8_t *>(
                                     tensor.data.virAddr);
          memcpy(tensor_data, img_data, data_length);
        }

        HB_SYS_flushMemCache(&tensor.data, HB_SYS_MEM_CACHE_CLEAN);
        break;
      }
      default:
        LOGE << "unsupport data_type: " << data_type;
        break;
    }
  }
  input_tensors_alloced_ = true;
}

void Predictor::PrepareOutputTensor() {
  if (output_tensors_alloced_) {
    return;
  }
  output_tensors_.resize(bpu_model_->output_num);
  for (int i = 0; i < bpu_model_->output_num; i++) {
    BPU_TENSOR_S &tensor = output_tensors_[i];
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
    HB_SYS_bpuMemAlloc("out_data0", output_size, true, &tensor.data);
  }
  output_tensors_alloced_ = true;
}

void Predictor::PrepareOutputTensorBatch(int batch_size) {
  if (batch_size > batch_size_) {
    if (output_tensors_alloced_) {
      ReleaseOutputTensor();
      output_tensors_alloced_ = false;
    }
    batch_size_ = batch_size + batch_size / 2;  // add extra space
  } else if (output_tensors_alloced_) {
    return;
  }
  output_tensors_.resize(bpu_model_->output_num);

  for (int i = 0; i < bpu_model_->output_num; i++) {
    BPU_TENSOR_S &tensor = output_tensors_[i];
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
    LOGD << "batch_size size: " << batch_size_;
    HB_SYS_bpuMemAlloc("out_data0", output_size * batch_size_, true,
                       &tensor.data);
  }
  output_tensors_alloced_ = true;
}

void Predictor::ReleaseInputTensor() {
  for (size_t i = 0; i < input_tensors_.size(); i++) {
    BPU_TENSOR_S *tensor = &input_tensors_[i];
    switch (tensor->data_type) {
      case BPU_TYPE_IMG_BGRP:
      case BPU_TYPE_IMG_RGBP:
      case BPU_TYPE_IMG_Y:
      case BPU_TYPE_IMG_RGB:
      case BPU_TYPE_IMG_BGR:
      case BPU_TYPE_IMG_YUV444:
      case BPU_TYPE_IMG_YUV_NV12:
      case BPU_TYPE_TENSOR_U8:
      case BPU_TYPE_TENSOR_S8:
      case BPU_TYPE_TENSOR_F32:
      case BPU_TYPE_TENSOR_S32:
      case BPU_TYPE_TENSOR_U32:
        HB_SYS_bpuMemFree(&(tensor->data));
        break;
      case BPU_TYPE_IMG_NV12_SEPARATE:
        HB_SYS_bpuMemFree(&(tensor->data));
        HB_SYS_bpuMemFree(&(tensor->data_ext));
      default:
        break;
    }
  }
}

void Predictor::ReleaseOutputTensor() {
  for (size_t i = 0; i < output_tensors_.size(); i++) {
    HB_SYS_bpuMemFree(&output_tensors_[i].data);
  }
  output_tensors_.clear();
}

int Predictor::RunModel(uint8_t *img_data,
                        int data_length,
                        BPU_DATA_TYPE_E data_type,
                        bool is_padding) {
  // 1. prepare input tensor
  PrepareInputTensor(img_data, data_length, data_type, is_padding);

  // 2. prepare output tensor
  PrepareOutputTensor();

  // 3. run model
  BPU_RUN_CTRL_S run_ctrl_s;
  run_ctrl_s.core_id = core_id_;
  BPU_TASK_HANDLE task_handle;
  int ret = HB_BPU_runModel(bpu_model_,
                            input_tensors_.data(),
                            bpu_model_->input_num,
                            output_tensors_.data(),
                            bpu_model_->output_num,
                            &run_ctrl_s,
                            false,
                            &task_handle);

  if (ret != 0) {
    LOGE << "bpu run model failed, " << HB_BPU_getErrorName(ret);
    // release input
    // ReleaseInputTensor();
    // release output
    // ReleaseOutputTensor();
    // release task_handle
    HB_BPU_releaseTask(&task_handle);
    return ret;
  }
  HB_BPU_waitModelDone(&task_handle);
  // 4. release input
  // ReleaseInputTensor();
  // 5. release BPU_TASK_HANDLE
  HB_BPU_releaseTask(&task_handle);

  return 0;
}

void Predictor::ConvertOutputToMXNet(void *src_ptr,
                                     void *dest_ptr,
                                     int out_index) {
  auto &aligned_shape = bpu_model_->outputs[out_index].aligned_shape;
  auto &real_shape = bpu_model_->outputs[out_index].shape;
  auto elem_size = 1;  // TODO(zhe.sun) 是否可判断float
  if (bpu_model_->outputs[out_index].data_type == BPU_TYPE_TENSOR_S32) {
    elem_size = 4;
  }
  auto shift = bpu_model_->outputs[out_index].shifts;

  uint32_t dst_n_stride =
      real_shape.d[1] * real_shape.d[2] * real_shape.d[3] * elem_size;
  uint32_t dst_h_stride = real_shape.d[2] * real_shape.d[3] * elem_size;
  uint32_t dst_w_stride = real_shape.d[3] * elem_size;
  uint32_t src_n_stride =
      aligned_shape.d[1] * aligned_shape.d[2] * aligned_shape.d[3] * elem_size;
  uint32_t src_h_stride = aligned_shape.d[2] * aligned_shape.d[3] * elem_size;
  uint32_t src_w_stride = aligned_shape.d[3] * elem_size;

  float tmp_float_value;
  int32_t tmp_int32_value;

  for (int nn = 0; nn < real_shape.d[0]; nn++) {
    void *cur_n_dst = reinterpret_cast<int8_t *>(dest_ptr) + nn * dst_n_stride;
    void *cur_n_src = reinterpret_cast<int8_t *>(src_ptr) + nn * src_n_stride;
    for (int hh = 0; hh < real_shape.d[1]; hh++) {
      void *cur_h_dst =
          reinterpret_cast<int8_t *>(cur_n_dst) + hh * dst_h_stride;
      void *cur_h_src =
          reinterpret_cast<int8_t *>(cur_n_src) + hh * src_h_stride;
      for (int ww = 0; ww < real_shape.d[2]; ww++) {
        void *cur_w_dst =
            reinterpret_cast<int8_t *>(cur_h_dst) + ww * dst_w_stride;
        void *cur_w_src =
            reinterpret_cast<int8_t *>(cur_h_src) + ww * src_w_stride;
        for (int cc = 0; cc < real_shape.d[3]; cc++) {
          void *cur_c_dst =
              reinterpret_cast<int8_t *>(cur_w_dst) + cc * elem_size;
          void *cur_c_src =
              reinterpret_cast<int8_t *>(cur_w_src) + cc * elem_size;
          if (elem_size == 4) {
            tmp_int32_value = *(reinterpret_cast<int32_t *>(cur_c_src));
            tmp_float_value = GetFloatByInt(tmp_int32_value, shift[cc]);
            *(reinterpret_cast<float *>(cur_c_dst)) = tmp_float_value;
          } else {
            *(reinterpret_cast<int8_t *>(cur_c_dst)) =
                *(reinterpret_cast<int8_t *>(cur_c_src));
          }
        }
      }
    }
  }
}

}  // namespace xstream
