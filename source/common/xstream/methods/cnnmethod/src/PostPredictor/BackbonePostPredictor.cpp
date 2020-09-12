/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @Author: xudong.du
 * @Email: xudong.du@horizon.ai
 * @Date: 2020-05-16 14:18:28
 * @Last Modified by: xudong.du
 * @Last Modified time: 2020-06-06 15:18:28
 */
#include <assert.h>
#include <vector>
#include <iostream>
#include "CNNMethod/PostPredictor/BackbonePostPredictor.h"
#include "CNNMethod/CNNConst.h"
#include "CNNMethod/util/util.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"
#include "horizon/vision_type/vision_type.h"

namespace xstream {

void BackBonePostPredictor::HandleBackboneInfo(
    const std::vector<std::vector<int8_t>> &mxnet_output,
    std::vector<BaseDataPtr> *output, CNNMethodRunData *run_data, int index) {
  assert(mxnet_output.size() == 1);
  assert(mxnet_output[0].size() == 256 * 4);
  assert(run_data->elem_size[index] == 4);

  float *output_vec_f = reinterpret_cast<float *>(malloc(sizeof(float) * 256));
  auto backbone_result =
      std::make_shared<XStreamData<HorizonVisionFloatArray>>();
  assert(run_data->real_nhwc[index].size() == 4);
  size_t output_size =
      run_data->real_nhwc[index][0] * run_data->real_nhwc[index][1] *
      run_data->real_nhwc[index][2] * run_data->real_nhwc[index][3];
  assert(mxnet_output[0].size() == output_size * 4);
  backbone_result->value.num = 256;
  for (size_t i = 0; i < output_size; i++) {
    float value_f = *(reinterpret_cast<const float *>(
        reinterpret_cast<const char *>(mxnet_output[0].data()) +
        i * run_data->elem_size[index]));
    // std::cout << value_f << " ";
    output_vec_f[i] = value_f;
  }
  backbone_result->value.values = output_vec_f;
  output->push_back(std::static_pointer_cast<BaseData>(backbone_result));
  // Modify element size from 4 to 1
  std::cout << std::endl;
}

void BackBonePostPredictor::Do(CNNMethodRunData *run_data) {
  int batch_size = run_data->input_dim_size.size();
  run_data->output.resize(batch_size);
  HOBOT_CHECK(batch_size == 1);
  for (int batch_idx = 0; batch_idx < batch_size; batch_idx++) {
    int box_num = run_data->input_dim_size[batch_idx];
    box_num = box_num > 1 ? 1 : box_num;
    auto &mxnet_output = run_data->mxnet_output[batch_idx];
    std::vector<BaseDataPtr> &batch_i_output = run_data->output[batch_idx];
    batch_i_output.resize(output_slot_size_);
    for (int i = 0; i < output_slot_size_; i++) {
      auto base_data_vector = std::make_shared<BaseDataVector>();
      batch_i_output[i] = base_data_vector;
    }
    {
      auto boxes = std::static_pointer_cast<BaseDataVector>(
          (*(run_data->input))[batch_idx][0]);

      for (int dim_idx = 0; dim_idx < box_num; dim_idx++) {
        HOBOT_CHECK(dim_idx == 0)
            << "box num: " << box_num << " dim idx: " << dim_idx;
        std::vector<BaseDataPtr> output;
        HandleBackboneInfo(mxnet_output[dim_idx], &output, run_data,
                           batch_idx * box_num + dim_idx);
        for (int i = 0; i < output_slot_size_; i++) {
          HOBOT_CHECK(output_slot_size_ == 1);
          auto output_slot_i_data =
              std::static_pointer_cast<BaseDataVector>(batch_i_output[i]);
          output_slot_i_data->datas_.push_back(output[i]);
          LOGD << "person num: " << output_slot_i_data->datas_.size();
        }
      }
    }
  }
}

}  // namespace xstream
