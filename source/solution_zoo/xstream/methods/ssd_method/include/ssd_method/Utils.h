// Copyright (c) 2020 Horizon Robotics.All Rights Reserved.
//
// The material in this file is confidential and contains trade secrets
// of Horizon Robotics Inc. This is proprietary information owned by
// Horizon Robotics Inc. No part of this work may be disclosed,
// reproduced, copied, transmitted, or used in any way for any purpose,
// without the express written permission of Horizon Robotics Inc.

#ifndef UTILS_UTILS_H_
#define UTILS_UTILS_H_

#include <cstdint>
#include <string>
#include <vector>

#include "bpu_predict/bpu_predict_extension.h"
#include "horizon/vision_type/vision_type.hpp"
#if defined(X2)
#include "./hb_vio_interface.h"
#endif
using hobot::vision::PymImageFrame;
/**
 * Align by 16
 */
#define ALIGN_16(v) ((v + (16 - 1)) / 16 * 16)

uint64_t GetCurrentTime();

int LoadModel(std::string &model_file, BPU_MODEL_S *bpu_model);
void PrepareOutputTensors(BPU_MODEL_S *bpu_model,
                          std::vector<BPU_TENSOR_S> &tensors,
                          int32_t max_bbox_size = 1);

void ReleaseBPUBuffer(std::vector<BPU_TENSOR_S> &tensors);
void ReleaseImageInputTensors(std::vector<BPU_TENSOR_S> &tensors);
void PrintModelInfo(BPU_MODEL_S *bpu_model);

void InitClsName(const std::string &cls_name_file,
                 std::vector<std::string> &cls_names);
#ifdef X3
void Image2BPUTensor(const PymImageFrame &info, int pyramid_layer,
                     BPU_TENSOR_S &tensor);
#endif
#ifdef X2
void Image2BPUTensor(const img_info_t &info, int pyramid_layer,
                     BPU_TENSOR_S &tensor);
#endif

#endif  // UTILS_UTILS_H_
