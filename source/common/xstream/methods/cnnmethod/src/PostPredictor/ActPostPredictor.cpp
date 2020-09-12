/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: ActPostPredictor.cpp
 * @Brief: definition of the ActPostPredictor
 * @Author: shiyu.fu
 * @Email: shiyu.fu@horizon.ai
 * @Date: 2020-05-25
 */

#include <memory>
#include <vector>
#include <string>
#include "CNNMethod/PostPredictor/ActPostPredictor.h"
#include "CNNMethod/CNNConst.h"
#include "CNNMethod/util/util.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"

using hobot::vision::BBox;
using hobot::vision::Attribute;

namespace xstream {

static void setVaule(std::vector<BaseDataPtr> &batch_output,
                     std::vector<BaseDataPtr> value) {
  if (batch_output.size() != value.size()) return;

  auto size = batch_output.size();

  for (std::size_t i = 0; i < size; ++i) {
    auto vPtr = value[i];
    auto data_vector =
        std::static_pointer_cast<BaseDataVector>(batch_output[i]);
    data_vector->datas_.push_back(vPtr);
  }
}

int32_t ActPostPredictor::Init(std::shared_ptr<CNNMethodConfig> config) {
  PostPredictor::Init(config);
  threshold_ = config->GetFloatValue("threshold");
  groups_str_ = config->GetSTDStringValue("merge_groups");
  std::string s_out_type = config->GetSTDStringValue("output_type");
  auto out_type = g_lmkseq_output_map.find(s_out_type);
  HOBOT_CHECK(out_type != g_lmkseq_output_map.end())
      << "output type " << s_out_type << " not support";
  output_type_ = out_type->second;
  target_group_ = config->GetIntValue("target_group_idx");
  size_t delimiter = std::string::npos;
  size_t sub_delimiter = std::string::npos;
  std::string group = "";
  std::vector<int> vec;
  if (groups_str_.length() > 0) {
    delimiter = groups_str_.find(";");
    while (delimiter != std::string::npos) {
      // one group
      group = groups_str_.substr(0, delimiter);
      // remove brackets
      group = group.substr(1, group.length() - 2);
      // remove current group from groups
      groups_str_ =
          groups_str_.substr(delimiter + 1, groups_str_.length() - delimiter);
      // string to int
      sub_delimiter = group.find(",");
      while (sub_delimiter != std::string::npos) {
        int index = std::stoi(group.substr(0, sub_delimiter));
        vec.push_back(index);
        group = group.substr(sub_delimiter + 1, group.length() - sub_delimiter);
        sub_delimiter = group.find(",");
      }
      int index = std::stoi(group);
      vec.push_back(index);
      merge_groups_.push_back(vec);
      delimiter = groups_str_.find(";");
      vec.clear();
    }
    if (groups_str_.length() != 0) {  // only one group
      group = groups_str_.substr(1, groups_str_.length() - 2);
      sub_delimiter = group.find(",");
      while (sub_delimiter != std::string::npos) {
        int index = std::stoi(group.substr(0, sub_delimiter));
        vec.push_back(index);
        group = group.substr(sub_delimiter + 1, group.length() - sub_delimiter);
        sub_delimiter = group.find(",");
      }
      int index = std::stoi(group);
      vec.push_back(index);
      merge_groups_.push_back(vec);
      vec.clear();
    }
  }
  HOBOT_CHECK(merge_groups_.size() != 0) << "Failed to parse merge groups";
  HOBOT_CHECK(merge_groups_[0].size() != 0) << "Empty group";
  auto en_score_avg_ = config->GetIntValue("en_score_avg", 0);
  if (en_score_avg_) {
    auto window_size = config->GetFloatValue("window_size");
    post_util_.Instance().Init(window_size, merge_groups_.size());
  }
  auto dump_output_feat_env = getenv("dump_output_feature");
  if (dump_output_feat_env) {
    en_dump_feature_ = std::stoi(dump_output_feat_env);
  } else {
    en_dump_feature_ = 0;
  }
  return 0;
}

void ActPostPredictor::Do(CNNMethodRunData *run_data) {
  int batch_size = run_data->input_dim_size.size();
  run_data->output.resize(batch_size);

  for (int batch_idx = 0; batch_idx < batch_size; batch_idx++) {
    int dim_size = run_data->input_dim_size[batch_idx];
    auto &input_data = (*(run_data->input))[batch_idx];
    auto rois = std::static_pointer_cast<BaseDataVector>(input_data[0]);
    auto disappeared_track_ids =
        std::static_pointer_cast<BaseDataVector>(input_data[2]);
    auto timestamp = static_cast<float *>(run_data->context);
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

      if (en_dump_feature_) {
        std::fstream raw_score_file, pred_ret_file;
        raw_score_file.open("act_raw_score.txt", std::ios_base::app);
        pred_ret_file.open("pred_ret.txt", std::ios_base::app);
        raw_score_file << "[";
        pred_ret_file << "[";
      }
      for (int dim_idx = 0; dim_idx < dim_size; dim_idx++) {  // loop target
        auto roi =
          std::static_pointer_cast<XStreamData<BBox>>(rois->datas_[dim_idx]);
        auto track_id = roi->value.id;
        auto &target_mxnet = mxnet_output[dim_idx];
        if (target_mxnet.size() == 0) {
          if (en_dump_feature_) {
            std::fstream raw_score_file, pred_ret_file;
            raw_score_file.open("act_raw_score.txt", std::ios_base::app);
            raw_score_file << "[]";
            pred_ret_file.open("pred_ret.txt", std::ios_base::app);
            pred_ret_file << "[-1]";
          }
          setVaule(batch_output, DefaultVaule(output_slot_size_));
        } else {
          setVaule(batch_output, TargetPro(target_mxnet, dim_idx, dim_size,
                                           track_id, timestamp));
        }
      }
      if (en_dump_feature_) {
        std::fstream raw_score_file, pred_ret_file;
        raw_score_file.open("act_raw_score.txt", std::ios_base::app);
        raw_score_file << "]\n";
        pred_ret_file.open("pred_ret.txt", std::ios_base::app);
        pred_ret_file << "]\n";
      }
      // post process util clean using disappeared track id
      post_util_.Instance().Clean(disappeared_track_ids);
    }
  }
  delete static_cast<float *>(run_data->context);
}

std::vector<BaseDataPtr> ActPostPredictor::DefaultVaule(int size) {
  std::vector<BaseDataPtr> def;
  auto def_val = std::make_shared<XStreamData<Attribute<int>>>();
  def_val->value.value = -1;
  def_val->value.score = 0;
  for (int i = 0; i < size; ++i) {
    def.push_back(std::static_pointer_cast<BaseData>(def_val));
  }
  return def;
}

std::vector<BaseDataPtr> ActPostPredictor::TargetPro(
    const std::vector<std::vector<int8_t>> &mxnet_outs,
    int dim_idx, int dim_size, int track_id, float* timestamp) {
  std::vector<BaseDataPtr> vals;
  // softmax, merge by group
  // fall: return target group, gesture: return max index
  for (size_t i = 0; i < mxnet_outs.size(); ++i) {
    auto mxnet_out = reinterpret_cast<const float *>(mxnet_outs[i].data());
    uint32_t model_output_size = mxnet_outs[i].size() / 4;
    std::vector<float> model_outs;
    model_outs.resize(model_output_size);
    float max_score = mxnet_out[0], sum_score = 0;
    std::fstream raw_score_file;
    if (en_dump_feature_) {
      std::fstream raw_score_file;
      raw_score_file.open("act_raw_score.txt", std::ios_base::app);
      raw_score_file << "[";
    }
    for (size_t idx = 0; idx < model_output_size; ++idx) {
      model_outs[idx] = mxnet_out[idx];
      if (en_dump_feature_) {
        std::fstream raw_score_file;
        raw_score_file.open("act_raw_score.txt", std::ios_base::app);
        raw_score_file << model_outs[idx];
        if (idx != model_output_size - 1) {
          raw_score_file << ", ";
        }
      }
      if (mxnet_out[idx] > max_score) {
        max_score = mxnet_out[idx];
      }
    }
    if (en_dump_feature_) {
      std::fstream raw_score_file;
      raw_score_file.open("act_raw_score.txt", std::ios_base::app);
      raw_score_file << "]";
      if (dim_idx != dim_size - 1) {
        raw_score_file << ", ";
      }
    }
    for (auto &item : model_outs) {
      item = std::exp(item - max_score);
      sum_score += item;
    }
    std::vector<float> act_rets(merge_groups_.size(), 0);
    float max_group_score = 0;
    size_t max_group_index = -1;
    if (en_score_avg_) {
      std::vector<float> tmp_rets(merge_groups_.size(), 0);
      for (size_t g_idx = 0; g_idx < merge_groups_.size(); ++g_idx) {
        for (size_t idx = 0; idx < merge_groups_[g_idx].size(); ++idx) {
          tmp_rets[g_idx] += model_outs[merge_groups_[g_idx][idx]] / sum_score;
        }
      }
      auto avg_rets =
        post_util_.Instance().GetCachedAvgScore(*timestamp, track_id, tmp_rets);
      for (size_t idx = 0; idx < avg_rets.size(); ++idx) {
        if (avg_rets[idx] > max_group_score) {
          max_group_score = avg_rets[idx];
          max_group_index = idx;
        }
      }
    } else {
      for (size_t g_idx = 0; g_idx < merge_groups_.size(); ++g_idx) {
        for (size_t idx = 0; idx < merge_groups_[g_idx].size(); ++idx) {
          act_rets[g_idx] += model_outs[merge_groups_[g_idx][idx]] / sum_score;
        }
        if (act_rets[g_idx] > max_group_score) {
          max_group_score = act_rets[g_idx];
          max_group_index = g_idx;
        }
      }
    }
    auto act_ret =
        std::make_shared<XStreamData<Attribute<int>>>();
    if (output_type_ == LmkSeqOutputType::FALL) {
      // fall detection result
      // 0: negative, 1: positive, -1: invalid
      act_ret->value.score = act_rets[target_group_];
      if (act_rets[target_group_] >= threshold_) {
        act_ret->value.value = 1;
      } else if (act_rets[target_group_] > 0 &&
                act_rets[target_group_] < threshold_) {
        act_ret->value.value = 0;
      } else {
        act_ret->value.value = -1;
      }
    } else if (output_type_ == LmkSeqOutputType::GESTURE) {
      // gesture recognition result
      // 0: No Gesture, 1: Pointing with One Finger, 2: Pointing with Two Finger
      // 3: Click with One Finger, 4: Clich with two Finger, 5: Throw Up
      // 6: Throw Down, 7: Throw Left, 8: Throw Right, 9: Open Twice
      // 10: Double Click with One Finger, 11: Double Click with Two Finger
      // 12: Zoom In, 13: Zoom Out
      if (max_group_score >= threshold_) {
        act_ret->value.value = max_group_index;
        act_ret->value.score = max_group_score;
      } else {
        act_ret->value.value = 0;
        act_ret->value.score = max_group_score;
      }
    }
    vals.push_back(std::static_pointer_cast<BaseData>(act_ret));
    LOGD << "idx: " << i << " act_value: " << act_ret->value.value
         << " act_score: " << act_ret->value.score;
    if (en_dump_feature_) {
      std::fstream pred_ret_file;
      pred_ret_file.open("pred_ret.txt", std::ios_base::app);
      pred_ret_file << "[" << act_ret->value.value << ", "
                    << act_ret->value.score << "]";
      if (dim_idx != dim_size - 1) {
        pred_ret_file << ", ";
      }
    }
  }
  return vals;
}

}  // namespace xstream
