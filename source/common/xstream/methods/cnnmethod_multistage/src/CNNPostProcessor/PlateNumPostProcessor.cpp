/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: PlateNumPostProcessor.h
 * @Brief: declaration of the PlateNumPostProcessor
 * @Author: yatao.fu
 * @Email: yatao.fu@horizon.ai
 * @Date: 2019-09-28 16:18:28
 * @Last Modified by: yatao.fu
 * @Last Modified time: 16:18:28
 */

#include "CNNPostProcessor/PlateNumPostProcessor.h"
#include "CNNConst.h"
#include "util/util.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"

using hobot::vision::BBox;

namespace xstream {
namespace CnnProc {
DEFINE_MethodCreator(plate_num_post, PlateNumPostProcessor())

int PlateNumPostProcessor::Init(const std::string &cfg_path) {
  CNNPostProcessor::Init(cfg_path);
  return 0;
}

std::vector<std::vector<BaseDataPtr>>
  PlateNumPostProcessor::DoProcess(
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
    auto &mxnet_output = mxnet_output_[batch_idx];
    std::vector<BaseDataPtr> &batch_output = outputs[batch_idx];
    batch_output.resize(output_slot_size_);

    auto &input_data = (run_data->input)[batch_idx];
    auto plate_types = std::static_pointer_cast<BaseDataVector>(input_data[2]);
    auto rois = std::static_pointer_cast<BaseDataVector>(input_data[0]);
    int dim_size = rois->datas_.size();

    for (int i = 0; i < output_slot_size_; i++) {
      auto base_data_vector = std::make_shared<BaseDataVector>();
      //  base_data_vector->name_ = output_slot_names_[i];
      batch_output[i] = std::static_pointer_cast<BaseData>(base_data_vector);
    }
    {
      RUN_PROCESS_TIME_PROFILER(model_name_ + "_post");
      RUN_FPS_PROFILER(model_name_ + "_post");
      auto data_vector =
          std::static_pointer_cast<BaseDataVector>(batch_output[0]);

      for (int dim_idx = 0, box_idx = 0; box_idx < dim_size; box_idx++) {
        // loop target
        auto plate_type =
        std::static_pointer_cast<XStreamData<hobot::vision::Attribute<int>>>(
                plate_types->datas_[box_idx]);
        auto vehiclePtr =
            std::make_shared<xstream::XStreamData<std::vector<int>>>();

        auto p_roi =
            std::static_pointer_cast<XStreamData<BBox>>(rois->datas_[box_idx]);
        if (p_roi->state_ != xstream::DataState::VALID) {
          vehiclePtr->value.clear();
        } else if (plate_type->value.value == 1) {
          auto &target_mxnet_up = mxnet_output[dim_idx++];
          auto &target_mxnet_down = mxnet_output[dim_idx++];
          // std::string str_up, str_down;
          std::vector<int> vec_up;
          std::vector<int> vec_down;

          if (target_mxnet_up.size() == 0) {
            vec_up.clear();
          } else {
            PlateNumPro(target_mxnet_up[0], vec_up);
          }
          if (target_mxnet_down.size() == 0) {
            vec_down.clear();
          } else {
            PlateNumPro(target_mxnet_down[0], vec_down);
          }

          if ((vec_up.size() == 0) || (vec_down.size() == 0)) {
            vehiclePtr->value.clear();
          } else {
            vehiclePtr->value.assign(vec_up.begin(), vec_up.end());
            vehiclePtr->value.insert(vehiclePtr->value.end(), vec_down.begin(),
                                     vec_down.end());
          }
        } else {
          auto &target_mxnet = mxnet_output[dim_idx++];
          if (target_mxnet.size() == 0) {
            vehiclePtr->value.clear();
          } else {
            PlateNumPro(target_mxnet[0], (vehiclePtr->value));
          }
        }
        // 车牌号只有7位或8位
        if ((vehiclePtr->value.size() != 7) &&
            (vehiclePtr->value.size() != 8)) {
          vehiclePtr->value.clear();
        }

        data_vector->datas_.push_back(vehiclePtr);
      }
    }
  }
  return outputs;
}

void PlateNumPostProcessor::PlateNumPro(const std::vector<int8_t> &mxnet_outs,
                                        std::vector<int> &platenum) {
  const int slice_num = 16;
  const int score_num = 74;
  if (mxnet_outs.size() != slice_num * score_num * sizeof(float)) {
    return;
  }

  auto mxnet_out = reinterpret_cast<const float *>(mxnet_outs.data());

  std::vector<int> scores;

  // get slice_num max index
  for (int index = 0; index < slice_num; index++) {
    int max_index = -1;
    float max_score = -9999.0f;
    for (int i = 0; i < score_num; ++i) {
      if (mxnet_out[index * score_num + i] > max_score) {
        max_score = mxnet_out[index * score_num + i];
        max_index = i;
      }
    }

    scores.push_back(max_index);
  }
  // skip the spilt/repeat character
  int last_index = -1;
  for (auto iter = scores.begin(); iter != scores.end();) {
    if (last_index == *iter || 0 == *iter) {
      last_index = *iter;
      iter = scores.erase(iter);
    } else {
      last_index = *iter;
      iter++;
    }
  }

  platenum.assign(scores.begin(), scores.end());

  return;
}  // namespace xstream

std::string PlateNumPostProcessor::PlateNumPro(
    const std::vector<int8_t> &mxnet_outs) {
  const int slice_num = 16;
  const int score_num = 74;
  if (mxnet_outs.size() != slice_num * score_num * sizeof(float)) {
    return "unknown";
  }

  auto mxnet_out = reinterpret_cast<const float *>(mxnet_outs.data());

  std::vector<int> scores;

  // get slice_num max index
  for (int index = 0; index < slice_num; index++) {
    int max_index = -1;
    float max_score = -9999.0f;

    for (int i = 0; i < score_num; ++i) {
      if (mxnet_out[index * score_num + i] > max_score) {
        max_score = mxnet_out[index * score_num + i];
        max_index = i;
      }
    }
    scores.push_back(max_index);
  }

  // skip the spilt/repeat character
  int last_index = -1;
  for (auto iter = scores.begin(); iter != scores.end();) {
    if (last_index == *iter || 0 == *iter) {
      last_index = *iter;
      iter = scores.erase(iter);
    } else {
      last_index = *iter;
      iter++;
    }
  }

  std::string plate_num;
  if ((scores.size() == 7 || scores.size() == 8) && scores[0] > 34 &&
      scores[1] >= 1 && scores[1] <= 24) {
    for (std::size_t i = 0; i < scores.size(); i++) {
      plate_num = plate_num + std::to_string(scores[i]) + "_";
    }
  } else {
    plate_num = "unknown";
  }

  return plate_num;
}
}  // namespace CnnProc
}  // namespace xstream
