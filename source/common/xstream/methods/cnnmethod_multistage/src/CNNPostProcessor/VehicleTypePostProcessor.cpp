/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: VehicleTypePostProcessor.cpp
 * @Brief: definition of the VehicleTypePostProcessor.h
 * @Author: yatao.fu
 * @Email: yatao.fu@horizon.ai
 * @Date: 2019-09-28 16:18:28
 * @Last Modified by: yatao.fu
 * @Last Modified time: 16:18:28
 */

#include "CNNPostProcessor/VehicleTypePostProcessor.h"
#include "CNNConst.h"
#include "util/util.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"

namespace xstream {
namespace CnnProc {
DEFINE_MethodCreator(vehicle_type_post, VehicleTypePostProcessor())

int VehicleTypePostProcessor::Init(const std::string &cfg_path) {
  CNNPostProcessor::Init(cfg_path);
  return 0;
}

std::vector<std::vector<BaseDataPtr>>
  VehicleTypePostProcessor::DoProcess(
      const std::vector<std::vector<BaseDataPtr> > &input,
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
    int dim_size = run_data->target_nums.size();
    auto &mxnet_output = mxnet_output_[batch_idx];
    std::vector<BaseDataPtr> &batch_output = outputs[batch_idx];
    batch_output.resize(output_slot_size_);
    auto &input_data = (run_data->input)[batch_idx];
    auto rois = std::static_pointer_cast<BaseDataVector>(input_data[0]);
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

      auto data_vector =
          std::static_pointer_cast<BaseDataVector>(batch_output[0]);

      for (int dim_idx = 0; dim_idx < dim_size; dim_idx++) {  // loop target
        auto vehiclePtr = std::make_shared<xstream::XStreamData<int>>();
        auto p_roi =
            std::static_pointer_cast<XStreamData<BBox>>(rois->datas_[dim_idx]);
#if 0
        if (log_file_.is_open()) {
          log_file_ << "box: " << p_roi->value.x1 << " " << p_roi->value.y1
                    << " " << p_roi->value.x2 << " " << p_roi->value.y2
                    << std::endl;
        }
#endif
        auto &target_mxnet = mxnet_output[dim_idx];

        if (target_mxnet.size() == 0) {
          vehiclePtr->value = -1;
        } else {
          vehiclePtr->value = VehicleTypePro(target_mxnet[0]);
        }
#if 0
        if (dim_idx == dim_size - 1) {
          if (log_file_.is_open()) {
            log_file_ << std::endl;
          }
        }
#endif

        data_vector->datas_.push_back(vehiclePtr);
      }
    }
  }
  return outputs;
}

int VehicleTypePostProcessor::VehicleTypePro(
    const std::vector<int8_t> &mxnet_outs) {
  if (mxnet_outs.size() != 12 * sizeof(float)) {
    return -1;
  }

  auto mxnet_out = reinterpret_cast<const float *>(mxnet_outs.data());
  float max_score = -9999.0f;
  int max_index = -1;

#if 0
  if (log_file_.is_open()) {
    log_file_ << "score: ";
  }
#endif

  for (int index = 0; index < 12; index++) {
#if 0
    if (log_file_.is_open()) {
      log_file_ << mxnet_out[index] << " ";
    }
#endif

    if (mxnet_out[index] > max_score) {
      max_score = mxnet_out[index];
      max_index = index;
    }
  }

#if 0
  if (log_file_.is_open()) {
    log_file_ << std::endl;
  }
#endif

  return max_index;
}
}  // namespace CnnProc
}  // namespace xstream
