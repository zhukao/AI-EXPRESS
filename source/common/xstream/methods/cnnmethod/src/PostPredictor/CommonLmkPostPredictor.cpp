/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: CommonLmkPostPredictor.h
 * @Brief: declaration of the CommonLmkPostPredictor
 * @Author: fei.cheng
 * @Email: fei.cheng@horizon.ai
 * @Date: 2020-07-18 14:18:28
 * @Last Modified by: fei.cheng
 * @Last Modified time: 2019-07-18 15:13:07
 */

#include "CNNMethod/PostPredictor/CommonLmkPostPredictor.h"
#include <vector>
#include "CNNMethod/CNNConst.h"
#include "CNNMethod/util/util.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"

namespace xstream {
int32_t CommonLmkPostPredictor::Init(std::shared_ptr<CNNMethodConfig> config) {
  if (nullptr == config) {
    return -1;
  }
  model_name_ = config->GetSTDStringValue("model_name");
  lmk_num_ = config->GetIntValue("lmk_num", 21);
  feature_w_ = config->GetIntValue("feature_w", 32);
  feature_h_ = config->GetIntValue("feature_h", 32);
  i_o_stride_ = config->GetIntValue("i_o_stride", 4);
  output_slot_size_ = config->GetIntValue("output_size");
  HOBOT_CHECK(output_slot_size_ > 0);
  return 0;
}

void CommonLmkPostPredictor::Do(CNNMethodRunData *run_data) {
  int batch_size = run_data->input_dim_size.size();
  run_data->output.resize(batch_size);
  for (int batch_idx = 0; batch_idx < batch_size; batch_idx++) {
    int dim_size = run_data->input_dim_size[batch_idx];
    auto &mxnet_output = run_data->mxnet_output[batch_idx];
    std::vector<BaseDataPtr> &batch_output = run_data->output[batch_idx];
    batch_output.resize(output_slot_size_);
    for (int i = 0; i < output_slot_size_; i++) {
      auto base_data_vector = std::make_shared<BaseDataVector>();
      batch_output[i] = std::static_pointer_cast<BaseData>(base_data_vector);
    }
    {
      RUN_PROCESS_TIME_PROFILER(model_name_ + "_post");
      RUN_FPS_PROFILER(model_name_ + "_post");
      auto boxes = std::static_pointer_cast<BaseDataVector>(
          (*(run_data->input))[batch_idx][0]);

      for (int dim_idx = 0; dim_idx < dim_size; dim_idx++) {
        std::vector<BaseDataPtr> output;
        auto xstream_box = std::static_pointer_cast<XStreamData<
                           hobot::vision::BBox>>(boxes->datas_[dim_idx]);
        HandleLmk(mxnet_output[dim_idx], xstream_box->value,
                      run_data->real_nhwc, run_data->all_shift, &output);

        for (int i = 0; i < output_slot_size_; i++) {
          auto base_data_vector =
              std::static_pointer_cast<BaseDataVector>(batch_output[i]);
          base_data_vector->datas_.push_back(output[i]);
        }
      }
    }
  }
}

void CommonLmkPostPredictor::HandleLmk(
    const std::vector<std::vector<int8_t>> &mxnet_outs,
    const hobot::vision::BBox &box,
    const std::vector<std::vector<uint32_t>> &nhwc,
    const std::vector<std::vector<uint32_t>> &shifts,
    std::vector<BaseDataPtr> *output) {
  if (mxnet_outs.size()) {
    auto lmk = Lmk3Post(mxnet_outs, box, nhwc, shifts);
    output->push_back(lmk);
  } else {
    auto landmarks = std::make_shared<XStreamData<hobot::vision::Landmarks>>();
    landmarks->state_ = DataState::INVALID;
    output->push_back(std::static_pointer_cast<BaseData>(landmarks));
  }
}

int CommonLmkPostPredictor::CalIndex(int k, int i, int j) {
  auto index = i * feature_w_ * lmk_num_ + \
          j * lmk_num_ + k;
  return index;
}

BaseDataPtr CommonLmkPostPredictor::Lmk3Post(
    const std::vector<std::vector<int8_t>> &mxnet_outs,
    const hobot::vision::BBox &box,
    const std::vector<std::vector<uint32_t>> &nhwc,
    const std::vector<std::vector<uint32_t>> &shifts) {
  auto heatmap_pred = reinterpret_cast<const int8_t *>(mxnet_outs[0].data());
  // static const float diff_coeff = 1.0;// for float32 of model output
  HOBOT_CHECK(shifts.size() > 0);
  HOBOT_CHECK(shifts[0].size() == lmk_num_);
  int raw_height = floor(box.Height());
  int raw_width = floor(box.Width());
  auto ratio_h = feature_h_ * i_o_stride_;
  auto ratio_w = feature_w_ * i_o_stride_;
#ifdef SEARCH_PERFORMANCE
  int step = 4;
#else
  int step = 1;
#endif
  auto landmarks = std::make_shared<XStreamData<hobot::vision::Landmarks>>();
  landmarks->value.values.resize(lmk_num_);
  for (size_t k = 0; k < lmk_num_; ++k) {  // c
    int8_t max_value = 0;
    int max_index[2] = {0, 0};
    for (auto i = 0; i < feature_h_; i += step) {  // h
      for (auto j = 0; j < feature_w_; j += step) {  // w
        int index = CalIndex(k, i, j);
        int8_t value = heatmap_pred[index];
        if (value > max_value) {
            max_value = value;
            max_index[0] = i;
            max_index[1] = j;
          }
        }  // w
      }  // h
#ifdef SEARCH_PERFORMANCE
      auto is_max = false;
      auto campare_func = [&max_index, &max_value, &is_max, heatmap_pred, this](
                              int i, int j, int k) {
        int index = CalIndex(k, i, j);
        if (max_value < heatmap_pred[index]) {
          max_value = heatmap_pred[index];
          max_index[0] = i;
          max_index[1] = j;
          is_max = false;
        }
      };
      while (false == is_max) {
        is_max = true;
        int i = max_index[0];
        int j = max_index[1];
        if (i > 0) {
          campare_func(i - 1, j, k);
        }
        if (i < feature_h_) {
          campare_func(i + 1, j, k);
        }
        if (j > 0) {
          campare_func(i, j - 1, k);
        }
        if (j < feature_w_) {
          campare_func(i, j + 1, k);
        }
      }
#endif
    float y = max_index[0];
    float x = max_index[1];

    float diff_x = 0, diff_y = 0;
    if (y > 0 && y < feature_h_ - 1) {
      int top = CalIndex(k, y-1, x);
      int down = CalIndex(k, y+1, x);
      diff_y = heatmap_pred[down] - heatmap_pred[top];
    }
    if (x > 0 && x < feature_w_ - 1) {
      int left = CalIndex(k, y, x-1);
      int right = CalIndex(k, y, x+1);
      diff_x = heatmap_pred[right] - heatmap_pred[left];
    }

    // y = y + (diff_y * diff_coeff + 0.5); // for float32 of model output
    // x = x + (diff_x * diff_coeff + 0.5); // for float32 of model output
    y = y + (diff_y > 0 ? 0.25 : -0.25) + 0.5;
    x = x + (diff_x > 0 ? 0.25 : -0.25) + 0.5;
    y = y * i_o_stride_;
    x = x * i_o_stride_;
    y = y * raw_height / ratio_h + box.y1;
    x = x * raw_width / ratio_w + box.x1;

    auto &poi = landmarks->value.values[k];
    poi.x = x;
    poi.y = y;
    float max_score = static_cast<float>(max_value);
    poi.score = max_score / (1 << shifts[0][k]);
    LOGD << "hand_lmk" << k << " x y score:( " << x << " " << y << " "
         << poi.score;
  }  // c

  return std::static_pointer_cast<BaseData>(landmarks);
}
}  // namespace xstream
