/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: CassifyPostPredictor.cpp
 * @Brief: definition of the CassifyPostPredictor
 * @Author: yatao.fu
 * @Email: yatao.fu@horizon.ai
 * @Date: 2019-09-28 16:18:28
 * @Last Modified by: yatao.fu
 * @Last Modified time: 16:18:28
 */

#include "CNNMethod/PostPredictor/ClassifyPostPredictor.h"
#include "CNNMethod/CNNConst.h"
#include "CNNMethod/util/util.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"

namespace xstream {

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

void ClassifyPostPredictor::Do(CNNMethodRunData *run_data) {
  int batch_size = run_data->input_dim_size.size();
  run_data->output.resize(batch_size);

  for (int batch_idx = 0; batch_idx < batch_size; batch_idx++) {
    int dim_size = run_data->input_dim_size[batch_idx];
    auto &mxnet_output = run_data->mxnet_output[batch_idx];
    std::vector<BaseDataPtr> &batch_output = run_data->output[batch_idx];
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
}

std::vector<int> ClassifyPostPredictor::DefaultVaule(int size) {
  std::vector<int> def;
  for (int i = 0; i < size; ++i) {
    def.push_back(-1);
  }
  return def;
}

std::vector<int> ClassifyPostPredictor::TargetPro(
    const std::vector<std::vector<int8_t>> &mxnet_outs) {
  std::vector<int> val;

  //  printf("the size is %d\n", mxnet_outs.size());
  for (std::size_t i = 0; i < mxnet_outs.size(); ++i) {
    auto mxnet_out = reinterpret_cast<const float *>(mxnet_outs[i].data());
    auto size = mxnet_outs[i].size() / 4;
    //    printf("the size 1 is %d\n", size);
    float max_score = -9999.0f;
    int max_index = -1;
    for (std::size_t index = 0; index < size; index++) {
      //      printf("mxnet_out[%d]:%f\n", index, mxnet_out[index]);
      if (mxnet_out[index] > max_score) {
        max_score = mxnet_out[index];
        max_index = index;
      }
    }
    val.push_back(max_index);
  }

  return val;
}

}  // namespace xstream
