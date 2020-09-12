/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: ClassifyPostProcessor.cpp
 * @Brief: definition of the ClassifyPostProcessor
 * @Author: yatao.fu
 * @Email: yatao.fu@horizon.ai
 * @Date: 2019-09-28 16:18:28
 * @Last Modified by: yatao.fu
 * @Last Modified time: 16:18:28
 */

#include <vector>
#include <memory>
#include <algorithm>
#include "CNNPostProcessor/ClassifyPostProcessor.h"
#include "CNNConst.h"
#include "util/util.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"

namespace xstream {
namespace CnnProc {
DEFINE_MethodCreator(classify_post, ClassifyPostProcessor())

static void setVaule(std::vector<BaseDataPtr> &batch_output,
                     std::vector<int> vaule) {
  if (batch_output.size() != vaule.size()) return;
  auto size = batch_output.size();

  for (std::size_t i = 0; i < size; ++i) {
    auto vPtr = std::make_shared<xstream::XStreamData<int>>();
    vPtr->value = vaule[i];
    auto data_vector =
        std::static_pointer_cast<BaseDataVector>(batch_output[i]);
    data_vector->datas_.push_back(vPtr);
  }
}

std::vector<std::vector<BaseDataPtr>>
ClassifyPostProcessor::DoProcess(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<xstream::InputParamPtr> &param) {
  HOBOT_CHECK(input.size() == 1) << "PostProcessor'input size must be 1";
  HOBOT_CHECK(input[0].size() == 1) << "PostProcessor'input size must be 1";

  std::shared_ptr<CNNPredictorOutputData> run_data;
  run_data = std::static_pointer_cast<CNNPredictorOutputData>(input[0][0]);
  getBPUResult(run_data);
  int batch_size = run_data->target_nums.size();

  std::vector<std::vector<BaseDataPtr>> outputs;
  outputs.resize(batch_size);

  for (int batch_idx = 0; batch_idx < batch_size; batch_idx++) {
    int dim_size = run_data->target_nums[batch_idx];
    auto &mxnet_output = mxnet_output_[batch_idx];
    std::vector<BaseDataPtr> &batch_output = outputs[batch_idx];
    batch_output.resize(output_slot_size_);
    for (int i = 0; i < output_slot_size_; i++) {
      auto base_data_vector = std::make_shared<BaseDataVector>();

#if 0
      base_data_vector->name_ = output_slot_names_[i];
#endif

      batch_output[i] = std::static_pointer_cast<BaseData>(base_data_vector);
    }
    {
      RUN_PROCESS_TIME_PROFILER(model_name_ + "_post");
      RUN_FPS_PROFILER(model_name_ + "_post");

      //      printf("the boxs is %d\n", dim_size);
      for (int dim_idx = 0; dim_idx < dim_size; dim_idx++) {  // loop target
        auto &target_mxnet = mxnet_output[dim_idx];
        if (target_mxnet.size() == 0) {
          setVaule(batch_output, DefaultVaule(output_slot_size_));
        } else {
          setVaule(batch_output, TargetPro(target_mxnet));
        }
      }
    }
  }
  return outputs;
}

std::vector<int> ClassifyPostProcessor::DefaultVaule(int size) {
  std::vector<int> def;
  for (int i = 0; i < size; ++i) {
    def.push_back(-1);
  }
  return def;
}

std::vector<int> ClassifyPostProcessor::TargetPro(
    const std::vector<std::vector<int8_t>> &mxnet_outs) {
  std::vector<int> val;
  for (std::size_t i = 0; i < mxnet_outs.size(); ++i) {
    auto mxnet_out = reinterpret_cast<const float *>(mxnet_outs[i].data());
    auto size = mxnet_outs[i].size() / 4;

    float max_score = -9999.0f;
    int max_index = -1;
    for (std::size_t index = 0; index < size; index++) {
      if (mxnet_out[index] > max_score) {
        max_score = mxnet_out[index];
        max_index = index;
      }
    }
    val.push_back(max_index);
  }

  return val;
}
}  // namespace CnnProc
}  // namespace xstream
