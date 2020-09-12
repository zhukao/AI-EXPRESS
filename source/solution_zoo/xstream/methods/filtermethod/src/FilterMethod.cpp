/*
 * @Description: face snap filter method
 * @Author:  hangjun.yang@horizon.ai
 * @Date: 2019-1-11 10:30:32
 * @Author: chao.yang@horizon.ai
 * @Date: 2019-5-27 10:30:32
 * @LastEditors  : hao.tian@horizon.ai
 * @LastEditTime : 2020-01-17 13:26:28
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */
#include "FilterMethod/FilterMethod.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "FilterMethod/filter_data_type.hpp"
#include "hobotlog/hobotlog.hpp"
#include "json/json.h"

namespace xstream {

int FilterMethod::Init(const std::string &config_file_path) {
  LOGI << "FilterMethod::Init " << config_file_path << std::endl;
  std::ifstream config_if(config_file_path);
  if (!config_if.good()) {
    LOGI << "FilterParam: no config, "
            "using default parameters"
         << std::endl;
    filter_param_ = std::make_shared<FilterParam>();
  } else {
    std::ostringstream buf;
    char ch;
    while (buf && config_if.get(ch)) {
      buf.put(ch);
    }
    filter_param_ = std::make_shared<FilterParam>(buf.str());
  }
  filter_param_->config_jv_all_ = filter_param_->config_jv;
  return 0;
}

std::vector<std::vector<BaseDataPtr>> FilterMethod::DoProcess(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<xstream::InputParamPtr> &param) {
  std::vector<std::vector<BaseDataPtr>> output;
  int batch_size = input.size();
  HOBOT_CHECK(batch_size > 0);
  output.resize(batch_size);
  for (int batch_idx = 0; batch_idx < batch_size; ++batch_idx) {
    auto &frame_i = input[batch_idx];
    auto &frame_o = output[batch_idx];
    auto &param_i = param[batch_idx];
    ProcessOneBatch(frame_i, &frame_o, param_i);
  }
  return output;
}

void FilterMethod::Finalize() {}

void FilterMethod::ProcessOneBatch(const std::vector<BaseDataPtr> &frame_i,
                                   std::vector<BaseDataPtr> *frame_o,
                                   const std::shared_ptr<InputParam> &param_i) {
  int input_size = frame_i.size();
  // at least boxes
  HOBOT_CHECK(input_size >= 1);

  // get box size & input slot
  std::size_t face_size = 0;
  std::size_t head_size = 0;
  std::size_t body_size = 0;
  std::size_t hand_size = 0;
  std::vector<BaseDataVectorPtr> input_slot;
  input_slot.resize(input_size);
  for (int i = 0; i < input_size; ++i) {
    auto data_info = filter_param_->slot_info_[i];
    auto data_slot = std::static_pointer_cast<BaseDataVector>(frame_i[i]);
    LOGD << "data name: " << data_info.name << " data type: " << data_info.type
         << " data group: " << data_info.group
         << " data size: " << data_slot->datas_.size();
    if (data_info.group == "face") {
      if (face_size <= 0) {
        face_size = data_slot->datas_.size();
      } else {
        HOBOT_CHECK(face_size == data_slot->datas_.size());
      }
    } else if (data_info.group == "head") {
      if (head_size <= 0) {
        head_size = data_slot->datas_.size();
      } else {
        HOBOT_CHECK(head_size == data_slot->datas_.size());
      }
    } else if (data_info.group == "body") {
      if (body_size <= 0) {
        body_size = data_slot->datas_.size();
      } else {
        HOBOT_CHECK(body_size == data_slot->datas_.size());
      }
    } else if (data_info.group == "hand") {
      if (hand_size <= 0) {
        hand_size = data_slot->datas_.size();
      } else {
        HOBOT_CHECK(hand_size == data_slot->datas_.size());
      }
    } else {
      LOGI << "Do not support this group now";
    }
    input_slot[i] = data_slot;
  }

  // generate the output slot, add copy the input data
  std::vector<BaseDataVectorPtr> output_slot;
  // the first slot used to describe the filter condition
  // the last slot used to descirbe bound rect
  output_slot.resize(input_size + 1 + 1);

  // decide whether do filter
  bool do_filter = true;

  if (param_i) {
    if (param_i->is_json_format_) {
      int param_ret = UpdateParameter(param_i);
      if (param_ret != 0) {
        LOGE << "filter input param update error";
        return;
      }
    }
    if (param_i->Format() == "pass-through") {
      LOGI << "pass-through mode";
      do_filter = false;
    }
  }

  if (do_filter) {
    LOGI << "filter mode";
    for (int i = 0; i < input_size; ++i) {
      HOBOT_CHECK("BaseDataVector" == frame_i[i]->type_) << "idx: " << i;
    }
    Copy2Output(input_slot, &output_slot, face_size, head_size, body_size,
                hand_size, false);
    // lost id erase and filter
    LostIdProcess(input_size, input_slot, output_slot);
    // attribute filter
    CommonFilter(input_size, face_size, "face", input_slot, output_slot);
    CommonFilter(input_size, head_size, "head", input_slot, output_slot);
    CommonFilter(input_size, body_size, "body", input_slot, output_slot);
    CommonFilter(input_size, hand_size, "hand", input_slot, output_slot);
    // max face count filter
    FaceCountFilter(input_size, face_size, &output_slot);
  } else {
    Copy2Output(input_slot, &output_slot, face_size, head_size, body_size,
                hand_size, true);
  }

  auto &frame_output = *frame_o;
  // compose output
  for (const auto &slot : output_slot) {
    frame_output.push_back(std::static_pointer_cast<BaseData>(slot));
  }
}

void FilterMethod::LostIdProcess(
    const int input_size, const std::vector<BaseDataVectorPtr> &input_slot,
    const std::vector<BaseDataVectorPtr> &output_slot) {
  for (int i = 0; i < input_size; i++) {
    auto data_info = filter_param_->slot_info_[i];
    auto data_size = input_slot[i]->datas_.size();
    LOGD << "data name: " << data_info.name << " data size: " << data_size
         << " idx: " << i;
    if ("id" == data_info.type) {
      for (size_t j = 0; j < data_size; j++) {
        auto lost_value =
            std::static_pointer_cast<XStreamID>(input_slot[i]->datas_[j]);
        auto lost_id = lost_value->value;
        if (filter_param_->stop_ids_.find(lost_id) !=
            filter_param_->stop_ids_.end()) {
          filter_param_->stop_ids_.erase(lost_id);
          LOGI << "erase id: " << lost_id;
        }
        if (filter_states_.find(lost_id) != filter_states_.end()) {
          auto &State = filter_states_[lost_id];
          if (State->is_filter_ && !State->is_normal_) {
            output_slot[i + 1]->datas_[j]->state_ =
                DataState(filter_param_->filter_status);
          }
          filter_states_.erase(lost_id);
        }
      }
    }
  }
}

int FilterMethod::GetIdByIdx(const int &input_size,
                             const std::vector<BaseDataVectorPtr> &input_slot,
                             const std::string &data_group,
                             const int &box_idx) {
  int32_t id = -1;
  for (int i = 0; i < input_size; i++) {
    auto data_info = filter_param_->slot_info_[i];
    auto data_size = input_slot[i]->datas_.size();
    if (data_group != data_info.group) {
      continue;
    }
    LOGD << "data name: " << data_info.name << " data type: " << data_info.type
         << " data group: " << data_info.group << " data size: " << data_size
         << " idx: " << box_idx;
    HOBOT_CHECK(box_idx < static_cast<int>(data_size));
    if ("bbox" == data_info.type) {
      auto bbox =
          std::static_pointer_cast<XStreamBBox>(input_slot[i]->datas_[box_idx]);
      if (bbox->state_ == DataState::VALID) {
        id = bbox->value.id;
      }
    } else {
      LOGD << "do not support this data type: " << data_info.type;
    }
  }
  return id;
}

void FilterMethod::ConstructState(const int &error_code, const int &state_flag,
                                  const int &box_id) {
  if (!(state_flag & filter_param_->enable_flag)) {
    if (filter_states_.find(box_id) == filter_states_.end()) {
      FilterStatePtr State(new FilterState());
      State->is_filter_ = true;
      filter_states_[box_id] = State;
    } else {
      auto &State = filter_states_[box_id];
      State->is_filter_ = true;
    }
  } else {
    if (error_code == filter_param_->passed_err_code) {
      if (filter_states_.find(box_id) == filter_states_.end()) {
        FilterStatePtr State(new FilterState());
        State->is_normal_ = true;
        filter_states_[box_id] = State;
      } else {
        auto &State = filter_states_[box_id];
        State->is_normal_ = true;
      }
    } else {
      if (filter_states_.find(box_id) == filter_states_.end()) {
        FilterStatePtr State(new FilterState());
        State->is_filter_ = true;
        filter_states_[box_id] = State;
      } else {
        auto &State = filter_states_[box_id];
        State->is_filter_ = true;
      }
    }
  }
}

void FilterMethod::CommonFilter(
    const int &input_size, const int &box_size, const std::string &data_group,
    const std::vector<BaseDataVectorPtr> &input_slot,
    const std::vector<BaseDataVectorPtr> &output_slot) {
  for (int box_idx = 0; box_idx < box_size; ++box_idx) {
    int error_code = 0;
    int layer_num = 0;
    int state_flag = 0;
    int box_id = 0;
    if ("face" == data_group) {
      error_code = FaceValid(input_size, input_slot, box_idx);
      layer_num = 0;
      state_flag = 0x01;
    } else if ("head" == data_group) {
      error_code = HeadValid(input_size, input_slot, box_idx);
      layer_num = 1;
      state_flag = 0x02;
    } else if ("body" == data_group) {
      error_code = BodyValid(input_size, input_slot, box_idx);
      layer_num = 2;
      state_flag = 0x04;
    } else if ("hand" == data_group) {
      error_code = HandValid(input_size, input_slot, box_idx);
      layer_num = 3;
      state_flag = 0x08;
    }

    LOGD << "group: " << data_group << " error code: " << error_code;
    box_id = GetIdByIdx(input_size, input_slot, data_group, box_idx);
    ConstructState(error_code, state_flag, box_id);
    if (error_code == filter_param_->passed_err_code) {
      continue;
    } else {
      auto data_layer = output_slot[0]->datas_[layer_num];
      auto layer_data = std::static_pointer_cast<BaseDataVector>(data_layer);
      auto description = layer_data->datas_[box_idx];
      description->state_ = DataState::FILTERED;
      auto xstream_description =
          dynamic_cast<XStreamFilterDescription *>(description.get());
      xstream_description->value = error_code;
      for (int i = 1; i < input_size + 1; ++i) {
        auto slot_info = filter_param_->slot_info_[i - 1];
        LOGD << "slot name: " << slot_info.name
             << " slot type:" << slot_info.type
             << " slot group: " << slot_info.group;
        if (data_group == slot_info.group) {
          output_slot[i]->datas_[box_idx]->state_ =
              DataState(filter_param_->filter_status);
        }
      }
    }
  }
}

void FilterMethod::FaceCountFilter(
    int input_size, int face_size,
    std::vector<BaseDataVectorPtr> *p_output_slot) const {
  auto &output_slot = *p_output_slot;
  int i = 0;
  if (filter_param_->max_face_box_counts) {
    for (i = 0; i < input_size; i++) {
      auto slot_info = filter_param_->slot_info_[i];
      if (slot_info.name == "face_box") {
        break;
      }
    }
    LOGD << "slot name: " << filter_param_->slot_info_[i].name
         << " face size: " << face_size << " idx: " << i
         << " output size: " << output_slot[i]->datas_.size();
    std::vector<size_t> sorted_indexs;
    for (auto index = 0; index < face_size; ++index) {
      if (output_slot[i + 1]->datas_[index]->state_ == DataState::VALID) {
        sorted_indexs.push_back(index);
      }
    }
    auto face_box_num = std::min(filter_param_->max_face_box_counts,
                                 static_cast<int>(sorted_indexs.size()));
    std::partial_sort(
        sorted_indexs.begin(), sorted_indexs.begin() + face_box_num,
        sorted_indexs.end(), [&output_slot, &i](size_t lhs, size_t rhs) {
          auto face_box_lhs = std::static_pointer_cast<XStreamBBox>(
              output_slot[i + 1]->datas_[lhs]);
          auto face_box_rhs = std::static_pointer_cast<XStreamBBox>(
              output_slot[i + 1]->datas_[rhs]);
          return face_box_lhs->value.Width() * face_box_lhs->value.Height() >
                 face_box_rhs->value.Width() * face_box_rhs->value.Height();
        });
    for (std::size_t face_idx = face_box_num; face_idx < sorted_indexs.size();
         ++face_idx) {
      auto data_layer = output_slot[0]->datas_[0];
      auto layer_data = std::static_pointer_cast<BaseDataVector>(data_layer);
      auto description = layer_data->datas_[sorted_indexs[face_idx]];
      description->state_ = DataState::FILTERED;
      auto xstream_description =
          dynamic_cast<XStreamFilterDescription *>(description.get());
      xstream_description->value = filter_param_->big_face_err_code;
      for (int i = 1; i < input_size + 1; ++i) {
        FILTER_LOG(sorted_indexs[face_idx], "big face mode")
        auto slot_info = filter_param_->slot_info_[i - 1];
        LOGD << "slot name: " << slot_info.name
             << " slot type: " << slot_info.type
             << " slot group: " << slot_info.group;
        if ("face" == slot_info.group) {
          output_slot[i]->datas_[sorted_indexs[face_idx]]->state_ =
              DataState(filter_param_->filter_status);
        }
      }
    }
  }
}

int FilterMethod::FaceValid(const int &input_size,
                            const std::vector<BaseDataVectorPtr> &input_slot,
                            int idx) {
  bool size_pass = true;
  bool score_pass = true;
  bool expand_pass = true;
  bool snap_area_pass = true;
  bool black_list_pass = true;
  bool white_list_pass = true;
  bool pos_pass = true;
  bool lmk_pass = true;
  bool upper_limit_pass = true;
  bool lower_limit_pass = true;
  bool range_pass = true;
  bool age_pass = true;
  bool id_pass = true;
  for (int i = 0; i < input_size; i++) {
    auto data_info = filter_param_->slot_info_[i];
    auto data_size = input_slot[i]->datas_.size();
    if ("face" != data_info.group) {
      continue;
    }
    LOGD << "data name: " << data_info.name << " data type: " << data_info.type
         << " data group: " << data_info.group << " data size: " << data_size
         << " idx: " << idx;
    HOBOT_CHECK(idx < static_cast<int>(data_size));
    if ("bbox" == data_info.type) {
      auto bbox =
          std::static_pointer_cast<XStreamBBox>(input_slot[i]->datas_[idx]);
      if (bbox->state_ == DataState::VALID) {
        float x1 = bbox->value.x1;
        float y1 = bbox->value.y1;
        float x2 = bbox->value.x2;
        float y2 = bbox->value.y2;
        float box_score = bbox->value.score;
        int32_t id = bbox->value.id;
        id_pass = !MatchStopId(id);
        size_pass =
            ReachSizeThreshold(x1, y1, x2, y2, filter_param_->face_size_thr);
        expand_pass =
            ExpandThreshold(&(bbox->value), filter_param_->face_expand_scale);
        score_pass =
            PassPostVerification(box_score, filter_param_->face_pv_thr);
        snap_area_pass =
            IsWithinSnapArea(x1, y1, x2, y2, filter_param_->image_width,
                             filter_param_->image_height);
        black_list_pass = !IsWithinBlackListArea(x1, y1, x2, y2);
        white_list_pass = IsWithinWhiteListArea(x1, y1, x2, y2);

        if (!id_pass) {
          FILTER_LOG(idx, "face stop id")
          return filter_param_->stop_id_err_code;
        }

        if (!black_list_pass) {
          FILTER_LOG(idx, "face blacklist area")
          return filter_param_->black_list_err_code;
        }
        if (!white_list_pass) {
          FILTER_LOG(idx, "face whitelist area")
          return filter_param_->white_list_err_code;
        }
        if (!snap_area_pass) {
          FILTER_LOG(idx, "face bound")
          return filter_param_->snap_area_err_code;
        }
        if (!expand_pass) {
          FILTER_LOG(idx, "face expand")
          return filter_param_->expand_thr_err_code;
        }
        if (!size_pass) {
          FILTER_LOG_VALUE(idx, "face size", std::min(x2 - x1, y2 - y1))
          return filter_param_->snap_size_thr_err_code;
        }
        if (!score_pass) {
          FILTER_LOG_VALUE(idx, "face confidence", box_score)
          return filter_param_->pv_thr_err_code;
        }
      }
    } else if ("Pose3D" == data_info.type) {
      float pitch = 0, yaw = 0, roll = 0;
      auto pose =
          std::static_pointer_cast<XStreamPose3D>(input_slot[i]->datas_[idx]);
      if (pose->state_ == DataState::VALID) {
        pitch = pose->value.pitch;
        yaw = pose->value.yaw;
        roll = pose->value.roll;
        pos_pass = ReachPoseThreshold(pitch, yaw, roll);
        if (!pos_pass) {
          std::string frontal_str = "pitch: " + std::to_string(pitch) +
                                    " yaw: " + std::to_string(yaw) +
                                    " roll: " + std::to_string(roll);
          FILTER_LOG_VALUE(idx, "face frontal area", frontal_str)
          return filter_param_->frontal_thr_err_code;
        }
      }
    } else if ("landmark" == data_info.type) {
      auto lmk = std::static_pointer_cast<XStreamLandmarks>(
          input_slot[i]->datas_[idx]);
      LOGE << "face filter method:";
      if (lmk->state_ == DataState::VALID) {
        lmk_pass = LmkVerification(lmk);
        if (!lmk_pass) {
          FILTER_LOG(idx, "face landmark")
          return filter_param_->lmk_thr_err_code;
        }
      }
    } else if ("age" == data_info.type) {
      auto age =
          std::static_pointer_cast<XStreamAge>(input_slot[i]->datas_[idx]);
      if (age->state_ == DataState::VALID) {
        age_pass = ReachAgeThreshold(age->value.value, age->value.min,
                                     age->value.max, age->value.score);
        if (!age_pass) {
          FILTER_LOG_VALUE(idx, data_info.name, age->value.value)
          FILTER_LOG_VALUE(idx, data_info.name, age->value.score)
          return filter_param_->age_err_code;
        }
      }
    } else if ("uppper_limit" == data_info.type) {
      float upper_score = 0;
      auto attribute = std::static_pointer_cast<XStreamAttribute>(
          input_slot[i]->datas_[idx]);
      auto limit_val = GetUpperLimitVal(data_info.name);
      if (attribute->state_ == DataState::VALID) {
        upper_score = attribute->value.score;
        upper_limit_pass = BelowUpperLimit(attribute->value.score, limit_val);
        if (!upper_limit_pass) {
          FILTER_LOG_VALUE(idx, data_info.name, upper_score)
          return GetErrCode(data_info.name);
        }
      }
    } else if ("lower_limit" == data_info.type) {
      float lower_score = 0;
      auto attribute = std::static_pointer_cast<XStreamAttribute>(
          input_slot[i]->datas_[idx]);
      auto limit_val = GetLowerLimitVal(data_info.name);
      if (attribute->state_ == DataState::VALID) {
        lower_score = attribute->value.score;
        lower_limit_pass = AboveLowerLimit(attribute->value.score, limit_val);
        if (!lower_limit_pass) {
          FILTER_LOG_VALUE(idx, data_info.name, lower_score)
          return GetErrCode(data_info.name);
        }
      }
    } else if ("range_limit" == data_info.type) {
      int range_val = 0;
      auto attribute = std::static_pointer_cast<XStreamAttribute>(
          input_slot[i]->datas_[idx]);
      auto range = GetRangeVal(data_info.name);
      if (attribute->state_ == DataState::VALID) {
        range_val = attribute->value.value;
        range_pass = BetweenRange(range_val, range.first, range.second);
        if (!range_pass) {
          FILTER_LOG_VALUE(idx, data_info.name, range_val)
          return GetErrCode(data_info.name);
        }
      }
    } else {
      LOGD << "Face do not support this data type: " << data_info.type;
    }
  }
  return filter_param_->passed_err_code;
}

int FilterMethod::HeadValid(const int &input_size,
                            const std::vector<BaseDataVectorPtr> &input_slot,
                            int idx) {
  bool size_pass = true;
  bool score_pass = true;
  bool expand_pass = true;
  bool snap_area_pass = true;
  bool black_list_pass = true;
  bool white_list_pass = true;
  bool id_pass = true;
  for (int i = 0; i < input_size; i++) {
    auto data_info = filter_param_->slot_info_[i];
    auto data_size = input_slot[i]->datas_.size();
    if ("head" != data_info.group) {
      continue;
    }
    LOGD << "data name: " << data_info.name << " data type: " << data_info.type
         << " data group: " << data_info.group << " data size: " << data_size
         << " idx: " << idx;
    HOBOT_CHECK(idx < static_cast<int>(data_size));
    if ("bbox" == data_info.type) {
      auto bbox =
          std::static_pointer_cast<XStreamBBox>(input_slot[i]->datas_[idx]);
      if (bbox->state_ == DataState::VALID) {
        float x1 = bbox->value.x1;
        float y1 = bbox->value.y1;
        float x2 = bbox->value.x2;
        float y2 = bbox->value.y2;
        float box_score = bbox->value.score;
        int32_t id = bbox->value.id;
        id_pass = !MatchStopId(id);
        size_pass =
            ReachSizeThreshold(x1, y1, x2, y2, filter_param_->head_size_thr);
        expand_pass =
            ExpandThreshold(&(bbox->value), filter_param_->head_expand_scale);
        score_pass =
            PassPostVerification(box_score, filter_param_->head_pv_thr);
        snap_area_pass =
            IsWithinSnapArea(x1, y1, x2, y2, filter_param_->image_width,
                             filter_param_->image_height);
        black_list_pass = !IsWithinBlackListArea(x1, y1, x2, y2);
        white_list_pass = IsWithinWhiteListArea(x1, y1, x2, y2);

        if (!id_pass) {
          FILTER_LOG(idx, "face stop id")
          return filter_param_->stop_id_err_code;
        }
        if (!black_list_pass) {
          FILTER_LOG(idx, "head blacklist area")
          return filter_param_->black_list_err_code;
        }
        if (!white_list_pass) {
          FILTER_LOG(idx, "head whitelist area")
          return filter_param_->white_list_err_code;
        }
        if (!snap_area_pass) {
          FILTER_LOG(idx, "head bound")
          return filter_param_->snap_area_err_code;
        }
        if (!expand_pass) {
          FILTER_LOG(idx, "head expand")
          return filter_param_->expand_thr_err_code;
        }
        if (!size_pass) {
          FILTER_LOG_VALUE(idx, "head size", std::min(x2 - x1, y2 - y1))
          return filter_param_->snap_size_thr_err_code;
        }
        if (!score_pass) {
          FILTER_LOG_VALUE(idx, "head confidence", box_score)
          return filter_param_->pv_thr_err_code;
        }
      }
    } else {
      LOGD << "Head do not support this data type: " << data_info.type;
    }
  }
  return filter_param_->passed_err_code;
}

int FilterMethod::BodyValid(const int &input_size,
                            const std::vector<BaseDataVectorPtr> &input_slot,
                            int idx) {
  bool size_pass = true;
  bool score_pass = true;
  bool expand_pass = true;
  bool snap_area_pass = true;
  bool black_list_pass = true;
  bool white_list_pass = true;
  bool id_pass = true;
  bool lmk_pass = true;
  for (int i = 0; i < input_size; i++) {
    auto data_info = filter_param_->slot_info_[i];
    auto data_size = input_slot[i]->datas_.size();
    if ("body" != data_info.group) {
      continue;
    }
    LOGD << "data name: " << data_info.name << " data type: " << data_info.type
         << " data group: " << data_info.group << " data size: " << data_size
         << " idx: " << idx;
    HOBOT_CHECK(idx < static_cast<int>(data_size));
    if ("bbox" == data_info.type) {
      auto bbox =
          std::static_pointer_cast<XStreamBBox>(input_slot[i]->datas_[idx]);
      if (bbox->state_ == DataState::VALID) {
        float x1 = bbox->value.x1;
        float y1 = bbox->value.y1;
        float x2 = bbox->value.x2;
        float y2 = bbox->value.y2;
        float box_score = bbox->value.score;
        int32_t id = bbox->value.id;
        id_pass = !MatchStopId(id);
        size_pass =
            ReachSizeThreshold(x1, y1, x2, y2, filter_param_->body_size_thr);
        expand_pass =
            ExpandThreshold(&(bbox->value), filter_param_->body_expand_scale);
        score_pass =
            PassPostVerification(box_score, filter_param_->body_pv_thr);
        snap_area_pass =
            IsWithinSnapArea(x1, y1, x2, y2, filter_param_->image_width,
                             filter_param_->image_height);
        black_list_pass = !IsWithinBlackListArea(x1, y1, x2, y2);
        white_list_pass = IsWithinWhiteListArea(x1, y1, x2, y2);

        if (!id_pass) {
          FILTER_LOG(idx, "face stop id")
          return filter_param_->stop_id_err_code;
        }
        if (!black_list_pass) {
          FILTER_LOG(idx, "body blacklist area")
          return filter_param_->black_list_err_code;
        }
        if (!white_list_pass) {
          FILTER_LOG(idx, "body whitelist area")
          return filter_param_->white_list_err_code;
        }
        if (!snap_area_pass) {
          FILTER_LOG(idx, "body bound")
          return filter_param_->snap_area_err_code;
        }
        if (!expand_pass) {
          FILTER_LOG(idx, "body expand")
          return filter_param_->expand_thr_err_code;
        }
        if (!size_pass) {
          FILTER_LOG_VALUE(idx, "body size", std::min(x2 - x1, y2 - y1))
          return filter_param_->snap_size_thr_err_code;
        }
        if (!score_pass) {
          FILTER_LOG_VALUE(idx, "body confidence", box_score)
          return filter_param_->pv_thr_err_code;
        }
      }
    } else if ("landmark" == data_info.type) {
      auto lmk = std::static_pointer_cast<XStreamLandmarks>(
          input_slot[i]->datas_[idx]);
      if (lmk->state_ == DataState::VALID) {
        lmk_pass = LmkVerification(lmk);
        if (!lmk_pass) {
          FILTER_LOG(idx, "body landmark")
          return filter_param_->lmk_thr_err_code;
        }
      }
    } else {
      LOGD << "Head do not support this data type: " << data_info.type;
    }
  }
  return filter_param_->passed_err_code;
}

int FilterMethod::HandValid(const int &input_size,
                            const std::vector<BaseDataVectorPtr> &input_slot,
                            int idx) {
  bool size_pass = true;
  bool score_pass = true;
  bool expand_pass = true;
  bool snap_area_pass = true;
  bool black_list_pass = true;
  bool white_list_pass = true;
  bool id_pass = true;
  bool lmk_pass = true;
  for (int i = 0; i < input_size; i++) {
    auto data_info = filter_param_->slot_info_[i];
    auto data_size = input_slot[i]->datas_.size();
    if ("hand" != data_info.group) {
      continue;
    }
    LOGD << "data name: " << data_info.name << " data type: " << data_info.type
         << " data group: " << data_info.group << " data size: " << data_size
         << " idx: " << idx;
    HOBOT_CHECK(idx < static_cast<int>(data_size));
    if ("bbox" == data_info.type) {
      auto bbox =
          std::static_pointer_cast<XStreamBBox>(input_slot[i]->datas_[idx]);
      if (bbox->state_ == DataState::VALID) {
        float x1 = bbox->value.x1;
        float y1 = bbox->value.y1;
        float x2 = bbox->value.x2;
        float y2 = bbox->value.y2;
        float box_score = bbox->value.score;
        int32_t id = bbox->value.id;
        id_pass = !MatchStopId(id);
        size_pass =
            ReachSizeThreshold(x1, y1, x2, y2, filter_param_->hand_size_thr);
        expand_pass =
            ExpandThreshold(&(bbox->value), filter_param_->hand_expand_scale);
        score_pass =
            PassPostVerification(box_score, filter_param_->hand_pv_thr);
        snap_area_pass =
            IsWithinSnapArea(x1, y1, x2, y2, filter_param_->image_width,
                             filter_param_->image_height);
        black_list_pass = !IsWithinBlackListArea(x1, y1, x2, y2);
        white_list_pass = IsWithinWhiteListArea(x1, y1, x2, y2);

        if (!id_pass) {
          FILTER_LOG(idx, "hand stop id")
          return filter_param_->stop_id_err_code;
        }
        if (!black_list_pass) {
          FILTER_LOG(idx, "hand blacklist area")
          return filter_param_->black_list_err_code;
        }
        if (!white_list_pass) {
          FILTER_LOG(idx, "hand whitelist area")
          return filter_param_->white_list_err_code;
        }
        if (!snap_area_pass) {
          FILTER_LOG(idx, "hand bound")
          return filter_param_->snap_area_err_code;
        }
        if (!expand_pass) {
          FILTER_LOG(idx, "hand expand")
          return filter_param_->expand_thr_err_code;
        }
        if (!size_pass) {
          FILTER_LOG_VALUE(idx, "hand size", std::min(x2 - x1, y2 - y1))
          return filter_param_->snap_size_thr_err_code;
        }
        if (!score_pass) {
          FILTER_LOG_VALUE(idx, "hand confidence", box_score)
          return filter_param_->pv_thr_err_code;
        }
      }
    } else if ("landmark" == data_info.type) {
      auto lmk = std::static_pointer_cast<XStreamLandmarks>(
          input_slot[i]->datas_[idx]);
      if (lmk->state_ == DataState::VALID) {
        lmk_pass = LmkVerification(lmk);
        if (!lmk_pass) {
          FILTER_LOG(idx, "hand landmark")
          return filter_param_->lmk_thr_err_code;
        }
      }
    } else {
      LOGD << "hand do not support this data type: " << data_info.type;
    }
  }
  return filter_param_->passed_err_code;
}

void FilterMethod::Copy2Output(const std::vector<BaseDataVectorPtr> &input_slot,
                               std::vector<BaseDataVectorPtr> *p_output_slot,
                               uint32_t face_size, uint32_t head_size,
                               uint32_t body_size, uint32_t hand_size,
                               bool pass_through) {
  auto input_size = input_slot.size();
  auto &output_slot = *p_output_slot;
  if (pass_through) {
    for (size_t i = 0; i < input_size; ++i) {
      output_slot[i + 1] = input_slot[i];
    }
  } else {
    output_slot[0] =
        ConstructFilterOutputSlot0(face_size, head_size, body_size, hand_size);
    for (size_t i = 0; i < input_size; i++) {
      auto slot_info = filter_param_->slot_info_[i];
      output_slot[i + 1] = CopySlot(input_slot[i], slot_info.type);
    }
  }
  output_slot[input_size + 1] = ConstructBoundOutputSlot();
}

BaseDataVectorPtr FilterMethod::CopySlot(const BaseDataVectorPtr &attr,
                                         const std::string &data_type) {
  BaseDataVectorPtr out = std::make_shared<BaseDataVector>();
  int attr_size = attr->datas_.size();
  if (0 == attr_size) {
    return out;
  }
  out->state_ = attr->state_;
  for (const auto &data : attr->datas_) {
    if ("bbox" == data_type) {
      auto actual_data = std::static_pointer_cast<XStreamBBox>(data);
      std::shared_ptr<XStreamBBox> copy_data = std::make_shared<XStreamBBox>();
      *copy_data = *actual_data;
      out->datas_.push_back(std::static_pointer_cast<BaseData>(copy_data));
    } else if ("Pose3D" == data_type) {
      auto actual_data = std::static_pointer_cast<XStreamPose3D>(data);
      std::shared_ptr<XStreamPose3D> copy_data =
          std::make_shared<XStreamPose3D>();
      *copy_data = *actual_data;
      out->datas_.push_back(std::static_pointer_cast<BaseData>(copy_data));
    } else if ("landmark" == data_type) {
      auto actual_data = std::static_pointer_cast<XStreamLandmarks>(data);
      std::shared_ptr<XStreamLandmarks> copy_data =
          std::make_shared<XStreamLandmarks>();
      *copy_data = *actual_data;
      out->datas_.push_back(std::static_pointer_cast<BaseData>(copy_data));
    } else if ("id" == data_type) {
      auto actual_data = std::static_pointer_cast<XStreamID>(data);
      std::shared_ptr<XStreamID> copy_data = std::make_shared<XStreamID>();
      *copy_data = *actual_data;
      out->datas_.push_back(std::static_pointer_cast<BaseData>(copy_data));
    } else if ("age" == data_type) {
      auto actual_data = std::static_pointer_cast<XStreamAge>(data);
      std::shared_ptr<XStreamAge> copy_data = std::make_shared<XStreamAge>();
      *copy_data = *actual_data;
      out->datas_.push_back(std::static_pointer_cast<BaseData>(copy_data));
    } else if ("upper_limit" == data_type || "lower_limit" == data_type ||
               "range" == data_type) {
      auto actual_data = std::static_pointer_cast<XStreamAttribute>(data);
      std::shared_ptr<XStreamAttribute> copy_data =
          std::make_shared<XStreamAttribute>();
      *copy_data = *actual_data;
      out->datas_.push_back(std::static_pointer_cast<BaseData>(copy_data));
    } else {
      LOGI << "Do not support this input data type: " << data_type;
    }
  }
  return out;
}

bool FilterMethod::MatchStopId(const int32_t &id) {
  if (filter_param_->stop_ids_.find(id) != filter_param_->stop_ids_.end()) {
    return true;
  }
  return false;
}

bool FilterMethod::ReachPoseThreshold(const float &pitch, const float &yaw,
                                      const float &roll) {
  if (filter_param_->filter_with_frontal_thr) {
    // adapter x1 config
    int32_t pose = 2000 - (yaw * yaw / 16 + pitch * pitch / 9) * 10;
    if (pose < -1999) {
      pose = -1999;
    }
    return pose > filter_param_->frontal_thr;
  }

  float pitch_, yaw_, roll_, a, b, c;
  if (filter_param_->frontal_pitch_thr == 0) {
    pitch_ = 0;
    a = 1;
    LOGV << "pitch filter off";
  } else {
    pitch_ = ValueNorm(-90.f, 90.f, pitch);
    a = filter_param_->frontal_pitch_thr;
  }
  if (filter_param_->frontal_yaw_thr == 0) {
    yaw_ = 0;
    b = 1;
    LOGV << "yaw filter off";
  } else {
    yaw_ = ValueNorm(-90.f, 90.f, yaw);
    b = filter_param_->frontal_yaw_thr;
  }
  if (filter_param_->frontal_roll_thr == 0) {
    roll_ = 0;
    c = 1;
    LOGV << "roll filter off";
  } else {
    roll_ = ValueNorm(-90.f, 90.f, roll);
    c = filter_param_->frontal_roll_thr;
  }
  HOBOT_CHECK(a != 0) << "frontal_pitch_thr could not equal to " << a;
  HOBOT_CHECK(b != 0) << "frontal_yaw_thr could not equal to " << b;
  HOBOT_CHECK(c != 0) << "frontal_roll_thr could not equal to " << c;
  float face_frontal = pitch_ * pitch_ / (a * a) + yaw_ * yaw_ / (b * b) +
                       roll_ * roll_ / (c * c);
  return face_frontal <= 1;
}

bool FilterMethod::ReachSizeThreshold(const float &x1, const float &y1,
                                      const float &x2, const float &y2,
                                      const int &size_thr) {
  float width = x2 - x1;
  float height = y2 - y1;
  LOGD << "width: " << width << " height: " << height
       << " size thr: " << size_thr;
  return (width > size_thr) && (height > size_thr);
}

bool FilterMethod::BelowUpperLimit(const float &score, const float &thr) {
  return score < thr;
}

bool FilterMethod::AboveLowerLimit(const float &score, const float &thr) {
  return score >= thr;
}

bool FilterMethod::BetweenRange(const float &val, const float &min,
                                const float &max) {
  return val >= min && val <= max;
}

bool FilterMethod::ReachAgeThreshold(const int32_t &val, const int32_t &min,
                                     const int32_t &max, const float &score) {
  return score >= filter_param_->age_thr && min >= filter_param_->age_min &&
         max <= filter_param_->age_max;
}

bool FilterMethod::PassPostVerification(const float &bbox_score,
                                        const float &pv_thr) {
  return bbox_score > pv_thr;
}

bool FilterMethod::LmkVerification(
    const std::shared_ptr<XStreamLandmarks> &lmk) {
  if (filter_param_->lmk_filter_num > 0) {
    int filter_num = 0;
    for (const auto &point : lmk->value.values) {
      LOGD << "point.score:" << point.score;
      LOGD << "filter_lmk_thr:" << filter_param_->lmk_thr;
      if (point.score < filter_param_->lmk_thr) {
        if (++filter_num >= filter_param_->lmk_filter_num) {
          return false;
        }
      }
    }
  } else {
    for (auto &point : lmk->value.values) {
      if (point.score < filter_param_->lmk_thr) {
        return false;
      }
    }
  }
  return true;
}

bool FilterMethod::IsWithinSnapArea(const float &x1, const float &y1,
                                    const float &x2, const float &y2,
                                    const int &image_width,
                                    const int &image_height) {
  return (x1 > filter_param_->bound_thr_w) &&
         ((image_width - x2) > filter_param_->bound_thr_w) &&
         (y1 > filter_param_->bound_thr_h) &&
         ((image_height - y2) > filter_param_->bound_thr_h);
}

bool FilterMethod::IsWithinBlackListArea(const float &x1, const float &y1,
                                         const float &x2, const float &y2) {
  if (filter_param_->black_list_module.GetAreaList().empty()) {
    return false;
  }
  for (auto &black_area : filter_param_->black_list_module.GetAreaList()) {
    if (filter_param_->black_list_module.IsInArea(x1, y1, x2, y2, black_area)) {
      return true;
    }
  }
  return false;
}

bool FilterMethod::IsWithinWhiteListArea(const float &x1, const float &y1,
                                         const float &x2, const float &y2) {
  if (filter_param_->white_list_module.GetAreaList().empty()) {
    return true;
  }

  for (auto &white_area : filter_param_->white_list_module.GetAreaList()) {
    if (filter_param_->white_list_module.IsInArea(x1, y1, x2, y2, white_area)) {
      return true;
    }
  }
  return false;
}

bool FilterMethod::ExpandThreshold(hobot::vision::BBox *box,
                                   const float &expand_scale) {
  auto ret =
      NormalizeRoi(box, expand_scale, filter_param_->e_norm_method,
                   filter_param_->image_width, filter_param_->image_height);
  return !ret;
}

int FilterMethod::UpdateParameter(InputParamPtr ptr) {
  if (ptr->is_json_format_) {
    std::string content = ptr->Format();
    return filter_param_->UpdateParameter(content);
  } else {
    HOBOT_CHECK(0) << "only support json format config";
    return -1;
  }
}

InputParamPtr FilterMethod::GetParameter() const { return filter_param_; }
BaseDataVectorPtr FilterMethod::ConstructBoundOutputSlot() {
  BaseDataVectorPtr out = std::make_shared<BaseDataVector>();
  auto bound_box = std::make_shared<XStreamBBox>();
  bound_box->value.x1 = filter_param_->bound_thr_w;
  bound_box->value.y1 = filter_param_->bound_thr_h;
  bound_box->value.x2 = filter_param_->image_width - filter_param_->bound_thr_w;
  bound_box->value.y2 =
      filter_param_->image_height - filter_param_->bound_thr_h;
  bound_box->value.id = 1;
  bound_box->value.score = 1.0;
  out->datas_.emplace_back(bound_box);
  auto show_rect = std::make_shared<XStreamData<bool>>();
  show_rect->value = filter_param_->show_bound_rect;
  LOGD << "show_rect: " << show_rect->value;
  out->datas_.emplace_back(show_rect);
  return out;
}

BaseDataVectorPtr FilterMethod::ConstructFilterOutputSlot0(
    const size_t &face_num, const size_t &head_num, const size_t &body_num,
    const size_t &hand_num) {
  BaseDataVectorPtr out = std::make_shared<BaseDataVector>();
  BaseDataVectorPtr face_out = std::make_shared<BaseDataVector>();
  BaseDataVectorPtr head_out = std::make_shared<BaseDataVector>();
  BaseDataVectorPtr body_out = std::make_shared<BaseDataVector>();
  BaseDataVectorPtr hand_out = std::make_shared<BaseDataVector>();

  for (size_t i = 0; i < face_num; i++) {
    auto data = std::make_shared<XStreamFilterDescription>();
    data->value = filter_param_->passed_err_code;
    face_out->datas_.emplace_back(data);
  }
  out->datas_.emplace_back(BaseDataPtr(face_out));

  for (size_t i = 0; i < head_num; i++) {
    auto data = std::make_shared<XStreamFilterDescription>();
    data->value = filter_param_->passed_err_code;
    head_out->datas_.emplace_back(data);
  }
  out->datas_.emplace_back(BaseDataPtr(head_out));

  for (size_t i = 0; i < body_num; i++) {
    auto data = std::make_shared<XStreamFilterDescription>();
    data->value = filter_param_->passed_err_code;
    body_out->datas_.emplace_back(data);
  }
  out->datas_.emplace_back(BaseDataPtr(body_out));
  for (size_t i = 0; i < hand_num; i++) {
    auto data = std::make_shared<XStreamFilterDescription>();
    data->value = filter_param_->passed_err_code;
    hand_out->datas_.emplace_back(data);
  }
  out->datas_.emplace_back(BaseDataPtr(hand_out));
  return out;
}

int FilterMethod::NormalizeRoi(hobot::vision::BBox *src, float norm_ratio,
                               NormMethod norm_method, uint32_t total_w,
                               uint32_t total_h) {
  if (std::fabs(norm_ratio - 1) < 0.00001) return 0;
  hobot::vision::BBox dst = *src;
  float box_w = dst.x2 - dst.x1;
  float box_h = dst.y2 - dst.y1;
  float center_x = (dst.x1 + dst.x2) / 2.0f;
  float center_y = (dst.y1 + dst.y2) / 2.0f;
  float w_new = box_w;
  float h_new = box_h;
  switch (norm_method) {
    case NormMethod::BPU_MODEL_NORM_BY_WIDTH_LENGTH: {
      w_new = box_w * norm_ratio;
      h_new = box_h + w_new - box_w;
      if (h_new <= 0) return -1;
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_WIDTH_RATIO:
    case NormMethod::BPU_MODEL_NORM_BY_HEIGHT_RATIO:
    case NormMethod::BPU_MODEL_NORM_BY_LSIDE_RATIO: {
      h_new = box_h * norm_ratio;
      w_new = box_w * norm_ratio;
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_HEIGHT_LENGTH: {
      h_new = box_h * norm_ratio;
      w_new = box_w + h_new - box_h;
      if (w_new <= 0) return -1;
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_LSIDE_LENGTH: {
      if (box_w > box_h) {
        w_new = box_w * norm_ratio;
        h_new = box_h + w_new - box_w;
        if (h_new <= 0) return -1;
      } else {
        h_new = box_h * norm_ratio;
        w_new = box_w + h_new - box_h;
        if (w_new <= 0) return -1;
      }
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_LSIDE_SQUARE: {
      if (box_w > box_h) {
        w_new = box_w * norm_ratio;
        h_new = w_new;
      } else {
        h_new = box_h * norm_ratio;
        w_new = h_new;
      }
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_DIAGONAL_SQUARE: {
      float diagonal = sqrt(pow(box_w, 2.0) + pow(box_h, 2.0));
      w_new = h_new = diagonal * norm_ratio;
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_NOTHING:
      break;
    default:
      return 0;
  }
  dst.x1 = center_x - w_new / 2;
  dst.x2 = center_x + w_new / 2;
  dst.y1 = center_y - h_new / 2;
  dst.y2 = center_y + h_new / 2;

  if (dst.x1 < 0 || dst.y1 < 0 || dst.x2 > total_w || dst.y2 > total_h) {
    return -1;
  }

  return 0;
}

float FilterMethod::GetUpperLimitVal(const std::string &name) {
  if (name == "blur") return filter_param_->quality_thr;
  if (name == "eye_abnormalities") return filter_param_->abnormal_thr;
  if (name == "mouth_abnormal") return filter_param_->abnormal_thr;
  if (name == "left_eye") return filter_param_->left_eye_occluded_thr;
  if (name == "right_eye") return filter_param_->right_eye_occluded_thr;
  if (name == "left_brow") return filter_param_->left_brow_occluded_thr;
  if (name == "right_brow") return filter_param_->right_brow_occluded_thr;
  if (name == "forehead") return filter_param_->forehead_occluded_thr;
  if (name == "left_cheek") return filter_param_->left_cheek_occluded_thr;
  if (name == "right_cheek") return filter_param_->right_cheek_occluded_thr;
  if (name == "nose") return filter_param_->nose_occluded_thr;
  if (name == "mouth") return filter_param_->mouth_occluded_thr;
  if (name == "jaw") return filter_param_->jaw_occluded_thr;
  return 0;
}

float FilterMethod::GetLowerLimitVal(const std::string &name) { return 0; }

std::pair<float, float> FilterMethod::GetRangeVal(const std::string &name) {
  std::pair<float, float> range{0, 0};
  if (name == "brightness") {
    range.first = filter_param_->brightness_min;
    range.second = filter_param_->brightness_max;
  }
  return range;
}

int FilterMethod::GetErrCode(const std::string &name) {
  if (name == "blur") return filter_param_->quality_thr_err_code;
  if (name == "brightness") return filter_param_->brightness_err_code;
  if (name == "eye_abnormalities") return filter_param_->abnormal_thr_err_code;
  if (name == "mouth_abnormal") return filter_param_->abnormal_thr_err_code;
  if (name == "left_eye") return filter_param_->left_eye_occluded_thr_err_code;
  if (name == "right_eye")
    return filter_param_->right_eye_occluded_thr_err_code;
  if (name == "left_brow")
    return filter_param_->left_brow_occluded_thr_err_code;
  if (name == "right_brow")
    return filter_param_->right_brow_occluded_thr_err_code;
  if (name == "forehead") return filter_param_->forehead_occluded_thr_err_code;
  if (name == "left_cheek")
    return filter_param_->left_cheek_occluded_thr_err_code;
  if (name == "right_cheek")
    return filter_param_->right_cheek_occluded_thr_err_code;
  if (name == "nose") return filter_param_->nose_occluded_thr_err_code;
  if (name == "mouth") return filter_param_->mouth_occluded_thr_err_code;
  if (name == "jaw") return filter_param_->jaw_occluded_thr_err_code;
  return 0;
}

}  // namespace xstream
