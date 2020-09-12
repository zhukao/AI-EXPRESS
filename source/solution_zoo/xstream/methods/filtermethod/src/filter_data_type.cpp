/*
 * @Description: data type and utils
 * @Author: chao.yang@horizon.ai
 * @Date: 2019-5-27 10:30:32
 * @LastEditors  : hao.tian@horizon.ai
 * @LastEditTime : 2020-01-16 15:03:19
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */
#include "FilterMethod/filter_data_type.hpp"

#include <assert.h>

#include "hobotlog/hobotlog.hpp"
#include "json/json.h"

namespace xstream {

const std::map<std::string, NormMethod> FilterParam::norm_method_map_ = {
    {"norm_by_width_length", NormMethod::BPU_MODEL_NORM_BY_WIDTH_LENGTH},
    {"norm_by_width_ratio", NormMethod::BPU_MODEL_NORM_BY_WIDTH_RATIO},
    {"norm_by_height_rario", NormMethod::BPU_MODEL_NORM_BY_HEIGHT_RATIO},
    {"norm_by_lside_ratio", NormMethod::BPU_MODEL_NORM_BY_LSIDE_RATIO},
    {"norm_by_height_length", NormMethod::BPU_MODEL_NORM_BY_HEIGHT_LENGTH},
    {"norm_by_lside_length", NormMethod::BPU_MODEL_NORM_BY_LSIDE_LENGTH},
    {"norm_by_lside_square", NormMethod::BPU_MODEL_NORM_BY_LSIDE_SQUARE},
    {"norm_by_diagonal_square", NormMethod::BPU_MODEL_NORM_BY_DIAGONAL_SQUARE},
    {"norm_by_nothing", NormMethod::BPU_MODEL_NORM_BY_NOTHING}};

void FilterParam::SetFilterParam() {
  SET_FILTER_METHOD_PARAM(config_jv, Bool, show_bound_rect);
  SET_FILTER_METHOD_PARAM(config_jv, Int, image_width);
  SET_FILTER_METHOD_PARAM(config_jv, Int, image_height);
  SET_FILTER_METHOD_PARAM(config_jv, Int, face_size_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Int, head_size_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Int, body_size_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Int, hand_size_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, frontal_pitch_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, frontal_yaw_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, frontal_roll_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, frontal_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Bool, filter_with_frontal_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, face_pv_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, head_pv_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, body_pv_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, hand_pv_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, quality_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, lmk_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Int, lmk_filter_num);
  SET_FILTER_METHOD_PARAM(config_jv, Int, bound_thr_w);
  SET_FILTER_METHOD_PARAM(config_jv, Int, bound_thr_h);
  SET_FILTER_METHOD_PARAM(config_jv, Int, max_face_box_counts);
  SET_FILTER_METHOD_PARAM(config_jv, Double, brightness_min);
  SET_FILTER_METHOD_PARAM(config_jv, Double, brightness_max);
  SET_FILTER_METHOD_PARAM(config_jv, Double, abnormal_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, face_expand_scale);
  SET_FILTER_METHOD_PARAM(config_jv, Double, head_expand_scale);
  SET_FILTER_METHOD_PARAM(config_jv, Double, body_expand_scale);
  SET_FILTER_METHOD_PARAM(config_jv, Double, hand_expand_scale);
  SET_FILTER_METHOD_PARAM(config_jv, Int, filter_status);
  SET_FILTER_METHOD_PARAM(config_jv, Int, age_min);
  SET_FILTER_METHOD_PARAM(config_jv, Int, age_max);
  SET_FILTER_METHOD_PARAM(config_jv, Double, age_thr);
  if (filter_with_frontal_thr) {
    LOGD << "pose filter with frontal_thr, thr is " << frontal_thr;
  } else {
    LOGD << "pose filter with pitch: " << frontal_pitch_thr
         << " yaw: " << frontal_yaw_thr << " roll: " << frontal_roll_thr;
  }
}

void FilterParam::SetNormParam() {
  std::string norm_method;
  SET_FILTER_METHOD_PARAM(config_jv, String, norm_method);
  if (norm_method.empty()) {
    LOGW << "not set norm_method";
  } else {
    auto iter = norm_method_map_.find(norm_method);
    if (iter == norm_method_map_.end()) {
      LOGW << "unknown norm_method:" << norm_method;
    } else {
      e_norm_method = iter->second;
    }
  }
  LOGD << "norm_method:" << norm_method;
}

void FilterParam::SetOccludeParam() {
  SET_FILTER_METHOD_PARAM(config_jv, Double, left_eye_occluded_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, right_eye_occluded_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, left_brow_occluded_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, right_brow_occluded_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, forehead_occluded_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, left_cheek_occluded_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, right_cheek_occluded_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, nose_occluded_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, mouth_occluded_thr);
  SET_FILTER_METHOD_PARAM(config_jv, Double, jaw_occluded_thr);
}

void FilterParam::SetFilterErrCode() {
  auto &desp = config_jv["err_description"];
  SET_FILTER_ERROR_CODE(desp, passed);
  SET_FILTER_ERROR_CODE(desp, snap_area);
  SET_FILTER_ERROR_CODE(desp, snap_size_thr);
  SET_FILTER_ERROR_CODE(desp, expand_thr);
  SET_FILTER_ERROR_CODE(desp, frontal_thr);
  SET_FILTER_ERROR_CODE(desp, pv_thr);
  SET_FILTER_ERROR_CODE(desp, quality_thr);
  SET_FILTER_ERROR_CODE(desp, lmk_thr);
  SET_FILTER_ERROR_CODE(desp, black_list);
  SET_FILTER_ERROR_CODE(desp, white_list);
  SET_FILTER_ERROR_CODE(desp, big_face);
  SET_FILTER_ERROR_CODE(desp, age);
  SET_FILTER_ERROR_CODE(desp, stop_id);
  SET_FILTER_ERROR_CODE(desp, brightness);
  SET_FILTER_ERROR_CODE(desp, abnormal_thr);
}

void FilterParam::SetOccludeErrCode() {
  auto &desp = config_jv["err_description"];
  SET_FILTER_ERROR_CODE(desp, left_eye_occluded_thr);
  SET_FILTER_ERROR_CODE(desp, right_eye_occluded_thr);
  SET_FILTER_ERROR_CODE(desp, left_brow_occluded_thr);
  SET_FILTER_ERROR_CODE(desp, right_brow_occluded_thr);
  SET_FILTER_ERROR_CODE(desp, forehead_occluded_thr);
  SET_FILTER_ERROR_CODE(desp, left_cheek_occluded_thr);
  SET_FILTER_ERROR_CODE(desp, right_cheek_occluded_thr);
  SET_FILTER_ERROR_CODE(desp, nose_occluded_thr);
  SET_FILTER_ERROR_CODE(desp, mouth_occluded_thr);
  SET_FILTER_ERROR_CODE(desp, jaw_occluded_thr);
}

void FilterParam::SetBlackArea() {
  float black_area_iou_thr = 0.5;
  auto jv_data = config_jv["black_area_iou_thr"];
  if (!jv_data.isNull()) {
    black_area_iou_thr = jv_data.asFloat();
  }
  jv_data = config_jv["black_area_list"];
  if (!jv_data.isNull()) {
    black_list_module.ClearAreaList();
    int black_area_size = jv_data.size();
    for (int i = 0; i < black_area_size; ++i) {
      auto one_area = jv_data[i];
      if (one_area.isNull()) {
        continue;
      }
      int data_size = one_area.size();
      assert(data_size == 4 || data_size == 5);
      int x1 = one_area[0].asInt();
      int y1 = one_area[1].asInt();
      int x2 = one_area[2].asInt();
      int y2 = one_area[3].asInt();
      float iou_thr = black_area_iou_thr;
      if (data_size == 5) {
        iou_thr = one_area[4].asFloat();
      }
      black_list_module.AddList(x1, y1, x2, y2, iou_thr);
      LOGW << "Add black list: "
           << "x1: " << x1 << " y1: " << y1 << " x2: " << x2 << " y2: " << y2;
    }
  }
}

void FilterParam::SetWhiteArea() {
  auto jv_data = config_jv["white_area_list"];
  if (!jv_data.isNull()) {
    white_list_module.ClearAreaList();
    int white_area_size = jv_data.size();
    for (int i = 0; i < white_area_size; ++i) {
      auto one_area = jv_data[i];
      if (one_area.isNull()) {
        continue;
      }
      std::size_t data_size = one_area.size();
      HOBOT_CHECK(data_size == 4);
      int x1 = one_area[0].asInt();
      int y1 = one_area[1].asInt();
      int x2 = one_area[2].asInt();
      int y2 = one_area[3].asInt();
      white_list_module.AddList(x1, y1, x2, y2);
      LOGW << "Add white list: "
           << "x1: " << x1 << " y1: " << y1 << " x2: " << x2 << " y2: " << y2;
    }
  }
}

void FilterParam::SetStopID() {
  SET_FILTER_METHOD_PARAM(config_jv, Int, stop_id);
  if (stop_id >= 0) {
    LOGD << "Insert stop id: " << stop_id;
    stop_ids_.insert(stop_id);
    stop_id = -1;
    config_jv_all_["stop_id"] = -1;
  }
  SET_FILTER_METHOD_PARAM(config_jv, Int, start_id);
  if (start_id >= 0) {
    LOGD << "Erase start id: " << start_id;
    stop_ids_.erase(start_id);
    start_id = -1;
    config_jv_all_["start_id"] = -1;
  }
  for (const auto &id : stop_ids_) {
    LOGD << "Exist id: " << id;
  }
}

void FilterParam::BuildInputSlotMap() {
  auto jv_data = config_jv["input_slot"];
  if (!jv_data.isNull()) {
    uint32_t input_size = jv_data.size();
    for (uint32_t i = 0; i < input_size; ++i) {
      auto slot_info = jv_data[i];
      if (!slot_info.isNull()) {
        SlotInfo info;
        info.name = slot_info["name"].asString();
        info.type = slot_info["type"].asString();
        info.group = slot_info["group"].asString();
        slot_info_.insert(std::make_pair(i, info));
      }
    }
  } else {
    slot_info_ = {{0, SlotInfo{"face_box", "bbox", "face"}},
                  {1, SlotInfo{"pose3D", "Pose3D", "face"}},
                  {2, SlotInfo{"landmark", "landmark", "face"}},
                  {3, SlotInfo{"blur", "upper_limit", "face"}},
                  {4, SlotInfo{"brightness", "range", "face"}},
                  {5, SlotInfo{"eye_abnormalities", "upper_limit", "face"}},
                  {6, SlotInfo{"mouth_abnormal", "upper_limit", "face"}},
                  {7, SlotInfo{"left_eye", "upper_limit", "face"}},
                  {8, SlotInfo{"right_eye", "upper_limit", "face"}},
                  {9, SlotInfo{"left_brow", "upper_limit", "face"}},
                  {10, SlotInfo{"right_brow", "upper_limit", "face"}},
                  {11, SlotInfo{"forehead", "upper_limit", "face"}},
                  {12, SlotInfo{"left_cheek", "upper_limit", "face"}},
                  {13, SlotInfo{"right_cheek", "upper_limit", "face"}},
                  {14, SlotInfo{"nose", "upper_limit", "face"}},
                  {15, SlotInfo{"mouse", "upper_limit", "face"}},
                  {16, SlotInfo{"jaw", "upper_limit", "face"}}};
    LOGW << "Do not have input slot order params, use default order";
  }
}

void FilterParam::BuildFilterState() {
  SET_FILTER_METHOD_PARAM(config_jv, String, merge_method);
  SET_FILTER_METHOD_PARAM(config_jv, Bool, head_enable);
  SET_FILTER_METHOD_PARAM(config_jv, Bool, face_enable);
  SET_FILTER_METHOD_PARAM(config_jv, Bool, body_enable);
  SET_FILTER_METHOD_PARAM(config_jv, Bool, hand_enable);
  if (config_jv.isMember("merge_method") &&
      config_jv["merge_method"].isString()) {
    if (merge_method == "head_face") {
      body_enable = false;
      hand_enable = false;
      enable_flag =
          face_enable | head_enable << 1 | body_enable << 2 | hand_enable << 3;
    } else if (merge_method == "head_body") {
      enable_flag =
          face_enable | head_enable << 1 | body_enable << 2 | hand_enable << 3;
    } else {
      LOGE << "Not support this merge method, use default config: "
           << merge_method;
    }
  }
}

int FilterParam::UpdateParameter(const std::string &content) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING error;
  std::shared_ptr<Json::CharReader> json_reader(builder.newCharReader());
  try {
    bool ret = json_reader->parse(
        content.c_str(), content.c_str() + content.size(), &config_jv, &error);
    //  update params
    SetFilterParam();
    SetOccludeParam();
    SetNormParam();
    //  update errorcode
    SetFilterErrCode();
    SetOccludeErrCode();
    //  update blackarea
    SetBlackArea();
    //  update whitearea
    SetWhiteArea();
    //  add stop id status
    SetStopID();
    //  set slot order
    BuildInputSlotMap();
    BuildFilterState();

    if (ret) {
      return 0;
    } else {
      return -1;
    }
  } catch (std::exception &e) {
    return -1;
  }
}

std::string FilterParam::Format() { return config_jv_all_.toStyledString(); }

}  // namespace xstream
