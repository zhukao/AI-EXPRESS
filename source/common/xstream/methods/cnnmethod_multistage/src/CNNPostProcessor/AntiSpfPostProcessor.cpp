/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: AntiSpfPostProcessor.cpp
 * @Brief: definition of the AntiSpfPostProcessor
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-07-17 14:27:05
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-07-17 15:18:10
 */

#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include "CNNPostProcessor/AntiSpfPostProcessor.h"
#include "util/util.h"
#include "CNNConst.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"

namespace xstream {
namespace CnnProc {
DEFINE_MethodCreator(antispf_post, AntiSpfPostProcessor())

int AntiSpfPostProcessor::Init(const std::string &cfg_path) {
  CNNPostProcessor::Init(cfg_path);
  anti_spf_threshold_ = config_->GetFloatValue("threshold");
  return 0;
}

int AntiSpfPostProcessor::UpdateParameter(xstream::InputParamPtr ptr) {
  CNNPostProcessor::UpdateParameter(ptr);
  if (config_->KeyExist("threshold")) {
    anti_spf_threshold_ = config_->GetFloatValue("threshold");
  }
  return 0;
}

std::vector<std::vector<BaseDataPtr>>
AntiSpfPostProcessor::DoProcess(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<xstream::InputParamPtr> &param) {
  HOBOT_CHECK(input.size() == 1) << "PostProcessor'input size must be 1";
  HOBOT_CHECK(input[0].size() == 1) << "PostProcessor'input size must be 1";

  std::shared_ptr<CNNPredictorOutputData> run_data;
  run_data = std::static_pointer_cast<CNNPredictorOutputData>(input[0][0]);

  std::vector<std::vector<BaseDataPtr>> outputs;
  getBPUResult(run_data);

  int batch_size = run_data->target_nums.size();
  outputs.resize(batch_size);
  for (int batch_idx = 0; batch_idx < batch_size; batch_idx++) {
    auto &norm_rois = run_data->targets_data[batch_idx];
    int dim_size = run_data->target_nums[batch_idx];
    auto &mxnet_output = mxnet_output_[batch_idx];
    std::vector<BaseDataPtr> &batch_output = outputs[batch_idx];
    batch_output.resize(output_slot_size_);
    for (int i = 0; i < output_slot_size_; i++) {
      auto base_data_vector = std::make_shared<BaseDataVector>();
      batch_output[i] = std::static_pointer_cast<BaseData>(base_data_vector);
    }
    {
      RUN_PROCESS_TIME_PROFILER(model_name_ + "_post");
      RUN_FPS_PROFILER(model_name_ + "_post");
      auto data_vector =
          std::static_pointer_cast<BaseDataVector>(batch_output[0]);
      auto norm_vector =
          std::static_pointer_cast<BaseDataVector>(batch_output[1]);
      for (int dim_idx = 0; dim_idx < dim_size; dim_idx++) {  // loop target
        BaseDataPtr anti_spf = FaceAntiSpfPostPro(
            mxnet_output[dim_idx],
            run_data->md_info->real_nhwc_[0][3]);
        data_vector->datas_.push_back(anti_spf);
        norm_vector->datas_.push_back(norm_rois[dim_idx]);
      }
    }
  }
  return outputs;
}

BaseDataPtr AntiSpfPostProcessor::FaceAntiSpfPostPro(
    const std::vector<std::vector<int8_t>> &mxnet_outs, int channel_size) {
  auto anti_spf = std::make_shared<
  XStreamData<hobot::vision::Attribute<int>>>();
  if (mxnet_outs.size() == 0 || mxnet_outs[0].size() == 0) {
    anti_spf->value.value = -1;
    anti_spf->value.score = 1.0f;
    anti_spf->state_ = DataState::INVALID;
  } else {
    auto mxnet_out = reinterpret_cast<const float *>(mxnet_outs[0].data());
    if (channel_size == 1) {
      anti_spf->value.score = SigMoid(mxnet_out[0]);
      anti_spf->value.value =
          anti_spf->value.score > anti_spf_threshold_ ? 1 : 0;
    } else {
      std::vector<float> attrs = {mxnet_out[0], mxnet_out[1]};
      SoftMax(attrs);
      anti_spf->value.score = attrs[1];
      anti_spf->value.value =
          anti_spf->value.score > anti_spf_threshold_ ? 1 : 0;
    }
  }
  return std::static_pointer_cast<BaseData>(anti_spf);
}
}  // namespace CnnProc
}  // namespace xstream
