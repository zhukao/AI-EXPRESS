/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: FaceIdPostPredictor.cpp
 * @Brief: definition of the FaceIdPostPredictor
 * @Author: ronghui.zhang
 * @Email: ronghui.zhang@horizon.ai
 * @Date: 2020-01-21 14:27:05
 * @Last Modified by: ronghui.zhang
 * @Last Modified time: 2020-01-20 15:18:10
 */

#include "CNNPostProcessor/FaceIdPostProcessor.h"
#include <vector>
#include <memory>
#include <algorithm>
#include "CNNConst.h"
#include "util/util.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"


namespace xstream {
namespace CnnProc {
DEFINE_MethodCreator(feature_post, FaceIdPostProcessor())

std::vector<std::vector<BaseDataPtr>>
FaceIdPostProcessor::DoProcess(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<xstream::InputParamPtr> &param) {
  HOBOT_CHECK(input.size() == 1) << "PostProcessor'input size must be 1";
  HOBOT_CHECK(input[0].size() == 1) << "PostProcessor'input size must be 1";

  std::shared_ptr<CNNPredictorOutputData> run_data_;
  run_data_ = std::static_pointer_cast<CNNPredictorOutputData>(input[0][0]);
  getBPUResult(run_data_);

  int batch_size = run_data_->target_nums.size();
  std::vector<std::vector<BaseDataPtr>> result;
  result.resize(batch_size);

  for (int batch_idx = 0; batch_idx < batch_size; batch_idx++) {
    auto &input_data = (run_data_->input)[batch_idx];
    auto snaps = std::static_pointer_cast<BaseDataVector>(input_data[0]);
    std::size_t person_num = snaps->datas_.size();
    uint32_t total_snap = run_data_->target_nums[batch_idx];
    auto &mxnet_output = mxnet_output_[batch_idx];
    std::vector<BaseDataPtr> &batch_output = result[batch_idx];
    batch_output.resize(output_slot_size_);
    for (int i = 0; i < output_slot_size_; i++) {
      auto base_data_vector = std::make_shared<BaseDataVector>();
      batch_output[i] = std::static_pointer_cast<BaseData>(base_data_vector);
    }
    auto data_vector =
        std::static_pointer_cast<BaseDataVector>(batch_output[0]);
    for (uint32_t person_idx = 0, g_snap_idx = 0; person_idx < person_num;
         person_idx++) {
      auto face_features = std::make_shared<BaseDataVector>();  // one person
      auto one_person_snaps =
          dynamic_cast<BaseDataVector *>(snaps->datas_[person_idx].get());
      if (!one_person_snaps) {
        continue;
      }
      for (uint32_t snap_idx = 0; snap_idx < one_person_snaps->datas_.size()
                                  && g_snap_idx < total_snap; snap_idx++) {
        RUN_PROCESS_TIME_PROFILER(model_name_ + "_post");
        RUN_FPS_PROFILER(model_name_ + "_post");
        auto face_feature = FaceFeaturePostPro(mxnet_output[g_snap_idx++]);
        face_features->datas_.push_back(face_feature);
      }
      data_vector->datas_.push_back(face_features);
    }
  }

  return result;
}

BaseDataPtr FaceIdPostProcessor::FaceFeaturePostPro(
    const std::vector<std::vector<int8_t>> &mxnet_outs) {
  if (mxnet_outs.size() == 0 || mxnet_outs[0].size() == 0) {
    auto feature_invalid =
    std::make_shared<XStreamData<hobot::vision::Feature>>();
    feature_invalid->state_ = DataState::INVALID;
    return std::static_pointer_cast<BaseData>(feature_invalid);
  }
  static const int kFeatureCnt = 128;
  auto mxnet_rlt = reinterpret_cast<const float *>(mxnet_outs[0].data());

  auto feature = std::make_shared<XStreamData<hobot::vision::Feature>>();
  feature->value.values.resize(kFeatureCnt);
  for (int i = 0; i < kFeatureCnt; i++) {
    feature->value.values[i] = mxnet_rlt[i];
  }

  l2_norm(feature->value.values, kFeatureCnt);
  return std::static_pointer_cast<BaseData>(feature);
}
}  // namespace CnnProc
}  // namespace xstream
