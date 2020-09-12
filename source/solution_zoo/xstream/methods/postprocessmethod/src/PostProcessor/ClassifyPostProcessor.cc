/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: ClassifyPostProcessor.cc
 * @Brief: definition of the ClassifyPostProcessor
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-28 14:28:17
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-28 16:19:08
 */

#include "PostProcessMethod/PostProcessor/ClassifyPostProcessor.h"
#include <vector>
#include <memory>
#include <set>
#include "bpu_predict/bpu_predict_extension.h"
#include "bpu_predict/bpu_predict.h"
#include "bpu_predict/bpu_parse_utils.h"
#include "bpu_predict/bpu_parse_utils_extension.h"
#include "hobotxstream/profiler.h"
#include "hobotxsdk/xstream_data.h"
#include "hobotlog/hobotlog.hpp"
#include "horizon/vision_type/vision_type.hpp"

namespace xstream {

std::vector<std::vector<BaseDataPtr>> ClassifyPostProcessor::Do(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<xstream::InputParamPtr> &param) {
  std::vector<std::vector<BaseDataPtr>> output;
  output.resize(input.size());
  for (size_t i = 0; i < input.size(); i++) {
    const auto &frame_input = input[i];
    auto &frame_output = output[i];
    RunSingleFrame(frame_input, frame_output);
  }
  return output;
}

void ClassifyPostProcessor::RunSingleFrame(
    const std::vector<BaseDataPtr> &frame_input,
    std::vector<BaseDataPtr> &frame_output) {
  LOGD << "ClassifyPostProcessor RunSingleFrame";
  HOBOT_CHECK(frame_input.size() == 1);

  for (size_t out_index = 0; out_index < method_outs_.size(); ++out_index) {
    frame_output.push_back(std::make_shared<xstream::BaseDataVector>());
  }

  auto async_data = std::static_pointer_cast<XStreamData<
      std::shared_ptr<hobot::vision::AsyncData>>>(frame_input[0]);
  if (async_data->value->bpu_model == nullptr ||
      async_data->value->task_handle == nullptr ||
      async_data->value->output_tensors == nullptr ||
      async_data->value->input_tensors == nullptr) {
    LOGE << "Invalid AsyncData";
    return;
  }

  bpu_model_ = std::static_pointer_cast<BPU_MODEL_S>(
      async_data->value->bpu_model);
  task_handle_ = std::static_pointer_cast<BPU_TASK_HANDLE>(
      async_data->value->task_handle);
  input_tensors_ = std::static_pointer_cast<std::vector<BPU_TENSOR_S>>(
      async_data->value->input_tensors);
  output_tensors_ = std::static_pointer_cast<std::vector<BPU_TENSOR_S>>(
      async_data->value->output_tensors);
  src_image_width_ = async_data->value->src_image_width;
  src_image_height_ = async_data->value->src_image_height;
  model_input_height_ = async_data->value->model_input_height;
  model_input_width_ = async_data->value->model_input_width;

  {
    RUN_PROCESS_TIME_PROFILER("DetectRunModel");
    RUN_FPS_PROFILER("DetectRunModel");

    HB_BPU_waitModelDone(task_handle_.get());
    // release input
    // ReleaseTensor(input_tensors_);
    // release BPU_TASK_HANDLE
    HB_BPU_releaseTask(task_handle_.get());
  }

  {
    RUN_PROCESS_TIME_PROFILER("DetectPostProcess");
    RUN_FPS_PROFILER("DetectPostProcess");

    // TODO(zhe.sun) postprocess

    // release output_tensors
    ReleaseTensor(output_tensors_);
  }
}

}  // namespace xstream
