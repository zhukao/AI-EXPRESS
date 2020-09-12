/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: FaceQualityPostProcessor.cpp
 * @Brief: definition of the FaceQualityPostProcessor
 * @Author: ronghui.zhang
 * @Date: 2019-09-26 21:38:28
 * @Last Modified by: ronghui.zhang
 * @Last Modified time: 2019-09-26 21:38:28
 */

#include "CNNPostProcessor/FaceQualityPostProcessor.h"
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include "CNNConst.h"
#include "util/util.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"

namespace xstream {
namespace CnnProc {
DEFINE_MethodCreator(quality_post, FaceQualityPostProcessor())

int32_t
FaceQualityPostProcessor::Init(const std::string &cfg_path) {
  CNNPostProcessor::Init(cfg_path);
  threshold_ = config_->GetFloatValue("threshold");
  return 0;
}

int FaceQualityPostProcessor::UpdateParameter(xstream::InputParamPtr ptr) {
  CNNPostProcessor::UpdateParameter(ptr);
  if (config_->KeyExist("threshold")) {
    threshold_ = config_->GetFloatValue("threshold");
  }
  return 0;
}

std::vector<std::vector<BaseDataPtr>>
FaceQualityPostProcessor::DoProcess(
    const std::vector<std::vector<BaseDataPtr> > &input,
    const std::vector<xstream::InputParamPtr> &param) {
  HOBOT_CHECK(input.size() == 1) << "PostProcessor'input size must be 1";
  HOBOT_CHECK(input[0].size() == 1) << "PostProcessor'input size must be 1";

  std::shared_ptr<CNNPredictorOutputData> run_data_;
  run_data_ = std::static_pointer_cast<CNNPredictorOutputData>(input[0][0]);
  int batch_size = run_data_->target_nums.size();
  std::vector<std::vector<BaseDataPtr>> result;
  result.resize(batch_size);
  getBPUResult(run_data_);

  for (int batch_idx = 0; batch_idx < batch_size; batch_idx++) {
    int dim_size = run_data_->target_nums[batch_idx];
    auto &mxnet_output = mxnet_output_[batch_idx];

    std::vector<BaseDataPtr> &batch_output = result[batch_idx];
    batch_output.resize(output_slot_size_);
    for (int i = 0; i < output_slot_size_; i++) {
      auto base_data_vector = std::make_shared<BaseDataVector>();
      batch_output[i] = std::static_pointer_cast<BaseData>(base_data_vector);
    }
    {
      RUN_PROCESS_TIME_PROFILER(model_name_ + "_post");
      RUN_FPS_PROFILER(model_name_ + "_post");
      for (int dim_idx = 0; dim_idx < dim_size; dim_idx++) {  // loop target
        std::vector<BaseDataPtr> output;
        FaceQualityPostPro(mxnet_output[dim_idx], &output);
        for (int slot_idx = 0; slot_idx < output_slot_size_; slot_idx++) {
          auto base_data_vector =
              std::static_pointer_cast<BaseDataVector>(batch_output[slot_idx]);
          base_data_vector->datas_.push_back(output[slot_idx]);
        }
      }
    }
  }
  return result;
}

void FaceQualityPostProcessor::FaceQualityPostPro(
    const std::vector<std::vector<int8_t>> &mxnet_outs,
    std::vector<BaseDataPtr> *output) {
  output->resize(output_slot_size_);
  // layer 1
  for (int slot_idx = 0; slot_idx < output_slot_size_ - 1; slot_idx++) {
    auto attribute =
        std::make_shared<XStreamData<hobot::vision::Attribute<int>>>();
    if (mxnet_outs.size() == 0 || mxnet_outs[0].size() == 0) {
      attribute->value.value = -1;
      attribute->value.score = 1.0f;
      attribute->state_ = DataState::INVALID;
    } else {
      auto mxnet_out = reinterpret_cast<const float *>(mxnet_outs[0].data());
      attribute->value.score = SigMoid(mxnet_out[slot_idx]);
      attribute->value.value = attribute->value.score < threshold_ ? 0 : 1;
    }
    int out_idx = slot_idx == 0 ? slot_idx : slot_idx + 1;
    (*output)[out_idx] = std::static_pointer_cast<BaseData>(attribute);
  }

  // layer 2
  auto attribute = std::make_shared<
  XStreamData<hobot::vision::Attribute<int>>>();
  if (mxnet_outs.size() == 0 || mxnet_outs[1].size() == 0) {
    attribute->value.value = -1;
    attribute->value.score = 1.0f;
    attribute->state_ = DataState::INVALID;
  } else {
    auto mxnet_out = reinterpret_cast<const float *>(mxnet_outs[1].data());
    std::vector<float> attrs = {
        mxnet_out[0], mxnet_out[1], mxnet_out[2], mxnet_out[3]};
    SoftMax(attrs);
    auto largest = std::max_element(std::begin(attrs), std::end(attrs));
    auto pos = std::distance(std::begin(attrs), largest);
    attribute->value.value = pos;
    attribute->value.score = attrs[pos];
  }
  (*output)[1] = std::static_pointer_cast<BaseData>(attribute);
}
}  // namespace CnnProc
}  // namespace xstream
