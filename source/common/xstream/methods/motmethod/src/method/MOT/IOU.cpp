/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     X1_IOU MOT Implementation
 * @author    chao.yang
 * @email     chao01.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.17
 */

#include <memory>
#include <fstream>
#include <string>

#include "MOTMethod/MOT/IOU.h"
#include "MOTMethod/data_type/mot_data_type.hpp"
#include "MOTMethod/error_code.h"
#include "iou_tracking/tracking.h"
#include "horizon/vision_type/vision_type.hpp"
#include "hobotlog/hobotlog.hpp"
#include "json/json.h"

#define INVALID_ID -1

namespace xstream {

typedef XStreamData<hobot::vision::BBox> XStreamBBox;

typedef XStreamData<uint32_t> XStreamUint32;

int IOU::MotInit(const std::string &config_file_path) {
  LOGI << "IOU Mot::Init " << config_file_path << std::endl;
  config_param_ = std::make_shared<IOUParam>();
  std::ifstream config_if(config_file_path);
  if (!config_if.good()) {
    LOGI << "IOUParam: no config, using default parameters" << std::endl;
  } else {
    std::ostringstream buf;
    char ch;
    while (buf && config_if.get(ch)) {
      buf.put(ch);
    }
    config_param_->UpdateParameter(buf.str());
  }
  return SetTrackingModuleData();
}

int IOU::SetTrackingModuleData() {
  auto &data = tracking_module_data_;
  if (data) {
    tracking_data_free(data);
    delete data;
  }
  data = new tracking_module_data_t();
  auto iou_config = GetConfig();
  data->update_no_target_predict = iou_config->update_no_target_predict;
  data->support_hungarian = iou_config->support_hungarian;
  data->need_check_merge = iou_config->need_check_merge;
  data->is_x1 = iou_config->device == "X1";
  data->max_track_target_num = iou_config->max_track_target_num;
  data->max_det_target_num = iou_config->max_det_target_num;
  data->max_osv_list = data->max_track_target_num * MAX_OSV_NUM;
  data->track_time_gap_eliminate =
      iou_config->vanish_frame_count * iou_config->time_gap;
  tracking_data_alloc(data);
  return XSTREAM_MOT_OK;
}

int IOU::Track(const std::vector<BaseDataPtr> &in,
               std::vector<BaseDataPtr> &out) {
  int ret = 0;
  bool abnormal_flag = false;
  // in[0] is face_head_rects
  HOBOT_CHECK(!in.empty());
  auto in_rects = std::static_pointer_cast<BaseDataVector>(in[0]);

  HOBOT_CHECK("BaseDataVector" == in_rects->type_);
  auto out_rects = std::make_shared<BaseDataVector>();
  auto out_disappeared_ids = std::make_shared<BaseDataVector>();

  out.push_back(std::static_pointer_cast<BaseData>(out_rects));
  out.push_back(std::static_pointer_cast<BaseData>(out_disappeared_ids));

  int input_num = RectMsg2Box(tracking_module_data_->box, in_rects.get());
  LOGD << "Input num: " << std::dec << input_num << std::endl;
  auto iou_config = GetConfig();
  if (input_num != 0)
  {
    if ( (ret = multi_track(tracking_module_data_,
                               tracking_module_data_->box,
                               input_num, iou_config->time_gap)) < 0 ) {
      abnormal_flag = true;
    }
  } else {
    multi_track_notarget(tracking_module_data_, 0, iou_config->time_gap);
  }

  if (abnormal_flag) {
     CreateAbnormalDisappearIds(tracking_module_data_,
                                out_disappeared_ids.get());
     SetTrackingModuleData();
     int num = RectMsg2Box(tracking_module_data_->box, in_rects.get());
     mot_states_.clear();
     frame_count_ = 0;
     LOGD << "Input num: " << std::dec << num << std::endl;
     multi_track(tracking_module_data_,
                               tracking_module_data_->box,
                               num, iou_config->time_gap);
  }

  frame_count_++;
  // transfer Track targets
  std::vector<uint32_t> ids;
  auto iou_param = GetConfig();
  if (iou_param->original_bbox) {
    auto track_cnt = TrackData2Rect_OriginalBBox(
        tracking_module_data_, &ids, in_rects->datas_, &out_rects->datas_);
    HOBOT_CHECK(input_num >= track_cnt);
  } else {
    TrackData2Rect_KalmanBBox(tracking_module_data_, &ids, &out_rects->datas_);
  }
  ret = UpdateState(frame_count_, ids, out_disappeared_ids);

  return ret;
}

int IOU::CreateAbnormalDisappearIds(tracking_module_data_t *tracking_data,
                                    BaseDataVector *out_disappeared_ids) {
  list_h_t *pos = nullptr, *next = nullptr;
  track_target_t *trk;
//  osv_t * osv = nullptr;
  int id_cnt = 0;
  list_for_each_safe(pos, next, &tracking_data->active_targets[0]) {
    trk = reinterpret_cast<track_target_t *>(pos);
//    osv = reinterpret_cast<osv_t *>(list_tail(&trk->osv_head));
    std::shared_ptr<XStreamUint32> track_id(new XStreamUint32());
    track_id->type_ = "Number";
    track_id->value = trk->id;
    track_id->state_ = DataState::DISAPPEARED;
    out_disappeared_ids->datas_.push_back(track_id);
    id_cnt++;
  }
  return id_cnt;
}

int IOU::RectMsg2Box(box_s *box, BaseDataVector *rects_msg) {
  uint num = 0;
  size_t size = rects_msg->datas_.size();
  LOGD << "Box size: " << size;

  for (size_t i = 0; i < size; i++) {
    auto &in_rect = rects_msg->datas_[i];
    // HOBOT_CHECK("BBox" == in_rect->type_);
    if (DataState::VALID != in_rect->state_)
      continue;

    auto bbox = std::static_pointer_cast<XStreamBBox>(in_rect);
    if (num >= tracking_module_data_->max_track_target_num) {
      break;
    }
    box->left = bbox->value.x1;
    box->top = bbox->value.y1;
    box->width = bbox->value.Width();
    box->height = bbox->value.Height();
    box->score = bbox->value.score;
    box->index = i;
    box->model_id = 0;
    box++;
    num++;
  }
  return num;
}

int IOU::TrackData2Rect_KalmanBBox(tracking_module_data_t *tracking_data,
                                   std::vector<uint32_t> *ids,
                                   std::vector<BaseDataPtr> *p_out_rects) {
  if (!ids) {
    return XSTREAM_MOT_ERR_PARAM;
  }
  list_h_t *pos = nullptr, *next = nullptr;
  track_target_t *trk;
  osv_t *osv = nullptr;
  int track_cnt = 0;
  list_for_each_safe(pos, next, &tracking_data->active_targets[0]) {
    trk = reinterpret_cast<track_target_t *>(pos);
    if (trk->existing_time_gap > 0) {
      continue;
    }
    osv = reinterpret_cast<osv_t *>(list_tail(&trk->osv_head));
    std::shared_ptr<XStreamBBox> bbox(new XStreamBBox());
    bbox->type_ = "BBox";
    bbox->value.id = trk->id;
    bbox->value.x1 = osv->estimated.left;
    bbox->value.y1 = osv->estimated.top;
    bbox->value.x2 = osv->estimated.right;
    bbox->value.y2 = osv->estimated.bottom;
    bbox->value.score = trk->score;
    ids->push_back(trk->id);
    p_out_rects->push_back(bbox);
    track_cnt++;
  }
  return XSTREAM_MOT_OK;
}

int IOU::TrackData2Rect_OriginalBBox(tracking_module_data_t *tracking_data,
                                     std::vector<uint32_t> *ids,
                                     const std::vector<BaseDataPtr> &in_rects,
                                     std::vector<BaseDataPtr> *p_out_rects) {
  if (!ids) {
    return XSTREAM_MOT_ERR_PARAM;
  }
  list_h_t *pos = nullptr, *next = nullptr;
  track_target_t *trk;
  osv_t *osv = nullptr;
  int track_cnt = 0;
  list_for_each_safe(pos, next, &tracking_data->active_targets[0]) {
    trk = reinterpret_cast<track_target_t *>(pos);
    if (trk->existing_time_gap > 0) {
      continue;
    }
    track_cnt++;  //  calculate the length for the active_targets[0] list
  }
  int input_num = in_rects.size();
  HOBOT_CHECK(input_num >= track_cnt)
      << " input num:" << input_num << " track count:" << track_cnt;
  ids->resize(input_num);
  p_out_rects->resize(input_num);
  std::vector<bool> updated(input_num, false);
  list_for_each_safe(pos, next, &tracking_data->active_targets[0]) {
    trk = reinterpret_cast<track_target_t *>(pos);
    if (trk->existing_time_gap > 0) {
      continue;
    }
    osv = reinterpret_cast<osv_t *>(list_tail(&trk->osv_head));
    std::shared_ptr<XStreamBBox> bbox(new XStreamBBox());
    bbox->type_ = "BBox";
    bbox->value.id = trk->id;
    bbox->value.x1 = osv->observed.left;
    bbox->value.y1 = osv->observed.top;
    bbox->value.x2 = osv->observed.right;
    bbox->value.y2 = osv->observed.bottom;
    bbox->value.score = trk->score;
    int idx = osv->observed.index;

    // check if idx is available
    HOBOT_CHECK(idx < input_num) << "index:" << idx
        << " input num:" << input_num << " track count:" << track_cnt;

    // check if the state of idx is default
    HOBOT_CHECK(!updated[idx]) << "index:" << idx
        << " input num:" << input_num << " track count:" << track_cnt;

    if (DataState::VALID == in_rects.at(idx)->state_)
      updated[idx] = true;

    ids->at(idx) = trk->id;
    p_out_rects->at(idx)  = bbox;
  }

  for (size_t idx = 0; idx < updated.size(); idx++) {
    auto idx_update = updated[idx];
    if (!idx_update) {
      ids->at(idx) = INVALID_ID;
      auto p_in_rect = std::static_pointer_cast<XStreamBBox>(in_rects[idx]);
      std::shared_ptr<XStreamBBox> bbox(new XStreamBBox());
      bbox->type_ = "BBox";
      bbox->state_ = DataState::INVALID;
      bbox->value = hobot::vision::BBox(p_in_rect->value.x1,
                                        p_in_rect->value.y1,
                                        p_in_rect->value.x2,
                                        p_in_rect->value.y2,
                                        p_in_rect->value.score,
                                        INVALID_ID);
      p_out_rects->at(idx) = bbox;
      LOGD << "filter bbox: " << idx;
    }
  }

  return track_cnt;
}

int IOU::UpdateState(const uint64_t &frame_id,
                     const std::vector<uint32_t> &ids,
                     IOU::BaseDataVectorPtr &disappeared_ids_msg) {
  for (auto &id : ids) {
      // add a new Track
      if (mot_states_.find(id) == mot_states_.end()) {
        StatePtr state(new State());
        state->start_ = frame_id;
        state->last_ = frame_id;
        state->count_ = 1;
        mot_states_[id] = state;
      } else {
        mot_states_[id]->last_ = frame_id;
        mot_states_[id]->count_++;
      }
  }
  auto iou_config = GetConfig();
  // remove the expired Track and generate disappeared id list
  auto iter = mot_states_.begin();
  while (iter != mot_states_.end()) {
    auto &state = iter->second;
    if (frame_count_ - state->last_ == iou_config->vanish_frame_count) {
      std::shared_ptr<XStreamUint32> track_id(new XStreamUint32());
      track_id->type_ = "Number";
      track_id->value = iter->first;
      track_id->state_ = DataState::DISAPPEARED;
      disappeared_ids_msg->datas_.push_back(track_id);
      iter = mot_states_.erase(iter);
      continue;
    } else {
      iter++;
    }
  }
  return XSTREAM_MOT_OK;
}

std::shared_ptr<IOUParam> IOU::GetConfig() {
  auto select_config =
      std::static_pointer_cast<IOUParam>(config_param_);
  return select_config;
}

void IOU::MotFinalize() {
  if (tracking_module_data_) {
    tracking_data_free(tracking_module_data_);
    delete tracking_module_data_;
    tracking_module_data_ = nullptr;
  }
}

void IOU::Reset() {
  SetTrackingModuleData();
  mot_states_.clear();
  frame_count_ = 0;
  LOGI << "Clear MOT states!";
}

int IOU::UpdateParameter(const std::string &content) {
  return config_param_->UpdateParameter(content);
}
}  // namespace xstream

