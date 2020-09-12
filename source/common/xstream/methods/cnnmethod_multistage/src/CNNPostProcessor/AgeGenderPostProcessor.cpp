/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: LmkPosePostPredictor.cpp
 * @Brief: definition of the LmkPosePostPredictor
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-07-17 14:27:05
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-07-17 15:18:10
 */

#include <vector>
#include <memory>
#include <algorithm>
#include "CNNPostProcessor/AgeGenderPostProcessor.h"
#include "util/util.h"
#include "CNNConst.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"

namespace xstream {
namespace CnnProc {
DEFINE_MethodCreator(agegender_post, AgeGenderPostProcessor())

std::vector<std::vector<BaseDataPtr>>
AgeGenderPostProcessor::DoProcess(
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

      auto boxes = std::static_pointer_cast<BaseDataVector>(
          (run_data->input)[batch_idx][0]);

      for (int dim_idx = 0; dim_idx < dim_size; dim_idx++) {
        std::vector<BaseDataPtr> output;
        HandleAgeGender(mxnet_output[dim_idx], &output);

        for (int i = 0; i < output_slot_size_; i++) {
          auto base_data_vector =
              std::static_pointer_cast<BaseDataVector>(batch_output[i]);
          base_data_vector->datas_.push_back(output[i]);
        }
      }
    }
  }
  return outputs;
}

void AgeGenderPostProcessor::HandleAgeGender(
    const std::vector<std::vector<int8_t>> &mxnet_outs,
    std::vector<BaseDataPtr> *output) {
  if (mxnet_outs.size()) {
    auto age = AgePostPro(mxnet_outs[0]);
    output->push_back(age);
    auto gender = GenderPostPro(mxnet_outs[1]);
    output->push_back(gender);
  } else {
    auto age = std::make_shared<XStreamData<hobot::vision::Age>>();
    age->state_ = DataState::INVALID;
    output->push_back(std::static_pointer_cast<BaseData>(age));

    auto gender = std::make_shared<XStreamData<hobot::vision::Gender>>();
    gender->state_ = DataState::INVALID;
    output->push_back(std::static_pointer_cast<BaseData>(gender));
  }
}

BaseDataPtr
AgeGenderPostProcessor::AgePostPro(const std::vector<int8_t> &mxnet_outs) {
  auto mxnet_out = reinterpret_cast<const float *>(mxnet_outs.data());
  auto age_result = std::make_shared<XStreamData<hobot::vision::Age>>();
  auto &age_class = age_result->value.value;
  age_class = 0;
  for (int age_i = 0; age_i < 14; age_i += 2) {
    if (mxnet_out[age_i] < mxnet_out[age_i + 1]) {
      age_class += 1;
    }
  }
  age_result->value.min = g_age_range[age_class * 2];
  age_result->value.max = g_age_range[age_class * 2 + 1];
  return std::static_pointer_cast<BaseData>(age_result);
}

BaseDataPtr
AgeGenderPostProcessor::GenderPostPro(const std::vector<int8_t> &mxnet_outs) {
  auto mxnet_out = reinterpret_cast<const float *>(mxnet_outs.data());
  auto gender_result = std::make_shared<XStreamData<hobot::vision::Gender>>();
  gender_result->value.value = mxnet_out[0] > 0.0f ? 1 : -1;
  gender_result->value.score = mxnet_out[0];
  return std::static_pointer_cast<BaseData>(gender_result);
}

}  // namespace CnnProc
}  // namespace xstream
