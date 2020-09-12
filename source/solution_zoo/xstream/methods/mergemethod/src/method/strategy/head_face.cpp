/*
 * @Description: implement of data_type
 * @Author: ruoting.ding@horizon.ai
 * @Date: 2018-10-21 17:49:26
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-11-20 11:54:59
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */
#include "MergeMethod/strategy/head_face.h"

#include <fstream>
#include <memory>
#include <string>

namespace xstream {

int HeadFaceParam::UpdateParameter(const JsonReaderPtr &reader) {
  LOGD << "HeadFaceParam update config: " << this;
  auto ret = MergeParam::UpdateParameter(reader);
  return ret;
}

std::vector<BaseDataPtr> HeadFaceStrategy::ProcessFrame(
    const std::vector<BaseDataPtr> &in, const InputParamPtr &param) {
  HOBOT_CHECK(!in.empty());
  std::vector<BaseDataPtr> out;
  if (param) {
    std::string content = param->Format();
    if (content == "pass-through") {
      LOGD << "pass through mode";
      PassThroughSingleFrame(in, &out);
    } else {
      HOBOT_CHECK(param->is_json_format_) << "json format error";
      LOGD << "normal mode";
      Json::CharReaderBuilder builder;
      builder["collectComments"] = false;
      JSONCPP_STRING error;
      std::shared_ptr<Json::CharReader> json_reader(builder.newCharReader());
      Json::Value out_jv;
      auto ret = json_reader->parse(
          content.c_str(), content.c_str() + content.size(), &out_jv, &error);
      auto reader = std::make_shared<JsonReader>(out_jv);
      ret = UpdateParameter(reader);
      HOBOT_CHECK(ret == kHorizonVisionSuccess) << "param error";
      RunSingleFrame(in, &out);
    }
  } else {
    RunSingleFrame(in, &out);
  }
  LOGI << "Head Face Mode, out size: " << out.size();
  return out;
}

void HeadFaceStrategy::Finalize() { LOGI << "HeadFaceStrategy Finalize"; }

int HeadFaceStrategy::UpdateParameter(const JsonReaderPtr &reader) {
  return (std::static_pointer_cast<HeadFaceParam>(merge_config_param_))
      ->UpdateParameter(reader);
}

std::shared_ptr<HeadFaceParam> HeadFaceStrategy::GetConfig() {
  auto config_ = std::static_pointer_cast<HeadFaceParam>(merge_config_param_);
  return config_;
}

void HeadFaceStrategy::RemoveDisappearedMotId(
    const Type &type, const BaseDataPtr &idPtr,
    const std::shared_ptr<BaseDataVector> &disappereadTrackIdPtr,
    IDRelationInfo *id_relation_info) {
  auto id_list = std::static_pointer_cast<BaseDataVector>(idPtr);
  for (size_t i = 0; i < id_list->datas_.size(); ++i) {
    auto id =
        std::static_pointer_cast<XStreamUint32>(id_list->datas_[i])->value;
    auto removed_id = id_relation_info->RemoveTrack(type, id);
    if (removed_id != -1) {
      auto track_id = std::make_shared<XStreamUint32>();
      track_id->value = removed_id;
      track_id->state_ = xstream::DataState::DISAPPEARED;
      disappereadTrackIdPtr->datas_.emplace_back(BaseDataPtr(track_id));
    }
  }
}

std::set<int> HeadFaceStrategy::GetIdSet(const BaseDataPtr &boxPtr) {
  std::set<int> cur_id_set;
  auto box_list = std::static_pointer_cast<BaseDataVector>(boxPtr);
  for (const auto &data : box_list->datas_) {
    if (data->state_ == xstream::DataState::VALID) {
      auto box = std::static_pointer_cast<XStreamBBox>(data);
      auto &id = box->value.id;
      cur_id_set.insert(id);
    }
  }
  return cur_id_set;
}

int HeadFaceStrategy::FindConflictTrackIdFromCurrentFrame(
    std::unordered_map<int, int> face_id2main_id,
    const std::set<int> &cur_face_id_set, const int main_id) {
  for (int iter : cur_face_id_set) {
    if (face_id2main_id.find(iter) == face_id2main_id.end()) continue;
    if (face_id2main_id[iter] == main_id) return iter;
  }
  return -1;
}

int HeadFaceStrategy::FindConflictTrackIDFromAllTrack(
    const std::unordered_map<int, int> &obj_id2main_id, const int main_id) {
  for (const auto &iter : obj_id2main_id) {
    if (iter.second == main_id) return iter.first;
  }
  return -1;
}

void HeadFaceStrategy::GetMainId(
    const BaseDataPtr &bboxPtr, const std::unordered_map<int, int> &merged_info,
    const std::shared_ptr<BaseDataVector> &mergedBBoxPtr) {
  auto box_list = std::static_pointer_cast<BaseDataVector>(bboxPtr);
  for (const auto &data : box_list->datas_) {
    auto pdata = std::static_pointer_cast<XStreamBBox>(data);
    auto bbox = std::make_shared<XStreamBBox>();
    *bbox = *pdata;
    auto &id = bbox->value.id;
    auto itr = merged_info.find(id);
    if (itr == merged_info.end()) {
      if (0 == GetConfig()->filtered_box_state_type)
        bbox->state_ = xstream::DataState::FILTERED;
      else if (1 == GetConfig()->filtered_box_state_type)
        bbox->state_ = xstream::DataState::INVALID;
      else
        bbox->state_ = xstream::DataState::FILTERED;
    } else {
      bbox->value.id = itr->second;
    }
    mergedBBoxPtr->datas_.emplace_back(BaseDataPtr(bbox));
  }
}

int HeadFaceStrategy::Init(std::shared_ptr<MergeParam> config) {
  box_offset_.x_offset_ratio = 0.0f;
  box_offset_.y_offset_ratio = 0.0f;
  merge_config_param_ = config;
  return kHorizonVisionSuccess;
}

void HeadFaceStrategy::PassThroughSingleFrame(
    const std::vector<BaseDataPtr> &frame_input,
    std::vector<BaseDataPtr> *frame_output) {
  HOBOT_CHECK_GE(frame_input.size(), uint(3));
  // output is box1
  frame_output->emplace_back(frame_input[0]);
  // output is box2
  frame_output->emplace_back(frame_input[2]);
  // output is disappeared_face_id
  frame_output->emplace_back(frame_input[1]);
  LOGD << "Process passthrough output";
}

void HeadFaceStrategy::RunSingleFrame(
    const std::vector<BaseDataPtr> &frame_input,
    std::vector<BaseDataPtr> *frame_output) {
  HOBOT_CHECK_GE(frame_input.size(), uint(4));
  const auto &face_box_list = frame_input[0];
  const auto &disappeared_face_id = frame_input[1];
  const auto &head_box_list = frame_input[2];
  const auto &disappeared_head_id = frame_input[3];
  HOBOT_CHECK_EQ(face_box_list->type_, "BaseDataVector");
  HOBOT_CHECK_EQ(disappeared_face_id->type_, "BaseDataVector");
  HOBOT_CHECK_EQ(head_box_list->type_, "BaseDataVector");
  HOBOT_CHECK_EQ(disappeared_head_id->type_, "BaseDataVector");
  // landmarks for Camera correction
  BaseDataPtr rgb_landmarks = nullptr;
  BaseDataPtr nir_landmarks = nullptr;
  if (frame_input.size() >= 6) {
    rgb_landmarks = frame_input[4];
    nir_landmarks = frame_input[5];
    HOBOT_CHECK_EQ(rgb_landmarks->type_, "BaseDataVector");
    HOBOT_CHECK_EQ(nir_landmarks->type_, "BaseDataVector");
  }

  BaseDataVectorPtr face_box_res = std::make_shared<BaseDataVector>();
  BaseDataVectorPtr head_box_res = std::make_shared<BaseDataVector>();
  BaseDataVectorPtr disappeared_track_id = std::make_shared<BaseDataVector>();

  ProduceOutput(face_box_list, head_box_list, face_box_res, head_box_res);

  RemoveDisappearedTrack(disappeared_face_id, disappeared_head_id,
                         disappeared_track_id);

  frame_output->emplace_back(BaseDataPtr(face_box_res));
  frame_output->emplace_back(BaseDataPtr(head_box_res));
  frame_output->emplace_back(BaseDataPtr(disappeared_track_id));
  LOGD << "Process HeadFaceStrategy output";
}

std::vector<std::pair<int, int>> HeadFaceStrategy::GetMatchedPairs(
    const BaseDataPtr &face_bbox, const BaseDataPtr &head_bbox,
    const float &threshold) {
  auto face_box_list = std::static_pointer_cast<BaseDataVector>(face_bbox);
  auto head_box_list = std::static_pointer_cast<BaseDataVector>(head_bbox);
  const auto face_bbox_num = face_box_list->datas_.size();
  const auto head_bbox_num = head_box_list->datas_.size();

  std::vector<std::pair<int, int>> id_pairs;
  std::vector<float> face_area;
  std::vector<bool> is_used(face_bbox_num, false);

  for (const auto &data : face_box_list->datas_) {
    auto bbox = std::static_pointer_cast<XStreamBBox>(data);
    auto area = bbox->value.Height() * bbox->value.Width();
    face_area.emplace_back(area);
  }

  for (size_t head_track_i = 0; head_track_i < head_bbox_num; ++head_track_i) {
    auto head_data = head_box_list->datas_[head_track_i];
    auto head_box = std::static_pointer_cast<XStreamBBox>(head_data);
    if (head_box->state_ != xstream::DataState::VALID) continue;
    if (head_bbox->state_ == xstream::DataState::VALID) {
      auto &head_track_id = head_box->value.id;
      int max_ind = -1;
      float max_ratio = 0;
      XStreamBBox inter_box;
      for (size_t face_track_i = 0; face_track_i < face_bbox_num;
           ++face_track_i) {
        if (is_used[face_track_i]) continue;
        auto face_data = face_box_list->datas_[face_track_i];
        if (face_data->state_ == xstream::DataState::VALID) {
          auto face_box = std::static_pointer_cast<XStreamBBox>(face_data);
          inter_box.value.x1 = (std::max)(
              face_box->value.x1,
              MoveCoord(head_box->value.x1, box_offset_.x_offset_ratio,
                        head_box->value.Width()));
          inter_box.value.x2 = (std::min)(
              face_box->value.x2,
              MoveCoord(head_box->value.x2, box_offset_.x_offset_ratio,
                        head_box->value.Width()));
          inter_box.value.y1 = (std::max)(
              face_box->value.y1,
              MoveCoord(head_box->value.y1, box_offset_.y_offset_ratio,
                        head_box->value.Height()));
          inter_box.value.y2 = (std::min)(
              face_box->value.y2,
              MoveCoord(head_box->value.y2, box_offset_.y_offset_ratio,
                        head_box->value.Height()));

          if (inter_box.value.x2 <= inter_box.value.x1 ||
              inter_box.value.y2 <= inter_box.value.y1) {
            continue;  // no intersection
          }
          auto area =
              (inter_box.value.Width() + 1) * (inter_box.value.Height() + 1);
          const float ratio = area / face_area[face_track_i];
          if (max_ind == -1 || ratio > max_ratio) {
            max_ind = face_track_i;
            max_ratio = ratio;
          }
        }
      }
      if (max_ratio > threshold && max_ind != -1) {
        is_used[max_ind] = true;
        auto face_data = face_box_list->datas_[max_ind];
        if (face_data->state_ == xstream::DataState::VALID) {
          auto face_box = std::static_pointer_cast<XStreamBBox>(face_data);
          auto face_track_id = face_box->value.id;
          id_pairs.emplace_back(face_track_id, head_track_id);
        }
      }
    }
  }
  return id_pairs;
}

void HeadFaceStrategy::MergeFaceHeadTrackID(const BaseDataPtr &face_bbox,
                                            const BaseDataPtr &head_bbox,
                                            const std::set<int> &face_id_set,
                                            const std::set<int> &head_id_set,
                                            IDRelationInfo *id_merge_info) {
  auto config = GetConfig();
  auto id_pairs =
      GetMatchedPairs(face_bbox, head_bbox, config->match_threshold);

  for (const auto &id_pair : id_pairs) {
    auto face_mot_id = id_pair.first;
    auto head_mot_id = id_pair.second;
    // if head_track_i and face_track_i not in
    auto face_iter = id_merge_info->GetFaceMap().find(id_pair.first);
    auto head_iter = id_merge_info->GetHeadMap().find(id_pair.second);
    if (face_iter == id_merge_info->GetFaceMap().end() &&
        head_iter == id_merge_info->GetHeadMap().end()) {  // no track history
      id_merge_info->AddFaceHead(face_mot_id, head_mot_id);
    } else if (face_iter == id_merge_info->GetFaceMap().end()) {
      // head_track_i in, so set the main_id of face_track_i with main_id of
      // head_track_i
      const int main_id = head_iter->second;
      int found_track = FindConflictTrackIdFromCurrentFrame(
          id_merge_info->GetFaceMap(), face_id_set, main_id);
      if (found_track != -1) {
        //  exist current main id, change head and face id
        auto conflict_id = id_merge_info->RemoveTrack(Type::Head, head_mot_id);
        if (conflict_id != -1) conflict_ids_.push_back(conflict_id);
        id_merge_info->AddFaceHead(face_mot_id, head_mot_id);
      } else {
        // check if there exist repeated track
        int repeated_track = FindConflictTrackIDFromAllTrack(
            id_merge_info->GetFaceMap(), main_id);
        if (repeated_track != -1) {
          auto conflict_id =
              id_merge_info->RemoveTrack(Type::Face, repeated_track);
          if (conflict_id != -1) conflict_ids_.push_back(conflict_id);
        }
        id_merge_info->AddNewTrack(Type::Face, face_mot_id, main_id);
      }
    } else if (head_iter == id_merge_info->GetHeadMap().end()) {
      // face_track_i in, so set the main_id of face_track_i with main_id of
      // face_track_i
      const int main_id = face_iter->second;
      int found_track = FindConflictTrackIdFromCurrentFrame(
          id_merge_info->GetHeadMap(), head_id_set, main_id);
      if (found_track != -1) {
        //  exist current main id, change head and face id
        auto conflict_id = id_merge_info->RemoveTrack(Type::Face, face_mot_id);
        if (conflict_id != -1) conflict_ids_.push_back(conflict_id);
        id_merge_info->AddFaceHead(face_mot_id, head_mot_id);
      } else {
        // check if there exist repeated track
        int repeated_track = FindConflictTrackIDFromAllTrack(
            id_merge_info->GetHeadMap(), main_id);
        if (repeated_track != -1) {
          auto conflict_id =
              id_merge_info->RemoveTrack(Type::Head, repeated_track);
          if (conflict_id != -1) conflict_ids_.push_back(conflict_id);
        }
        id_merge_info->AddNewTrack(Type::Head, head_mot_id, main_id);
      }
    } else {
      // check if main_ids of head_track_i and face_track_i are same
      if (face_iter->second != head_iter->second) {
        // std::cout << " history no mapped" << std::endl;
        auto conflict_id = id_merge_info->RemoveTrack(Type::Head, head_mot_id);
        if (conflict_id != -1) conflict_ids_.push_back(conflict_id);
        id_merge_info->AddNewTrack(Type::Head, head_mot_id, face_iter->second);
      } else {
        // use original main id
      }
    }
  }
  // find unmatched id
  auto box_list = std::static_pointer_cast<BaseDataVector>(face_bbox);
  for (const auto &data : box_list->datas_) {
    auto box = std::static_pointer_cast<XStreamBBox>(data);
    if (box->state_ == xstream::DataState::VALID) {
      auto &track_id = box->value.id;
      auto iter = id_merge_info->GetFaceMap().find(track_id);
      if (iter == id_merge_info->GetFaceMap().end()) {
        id_merge_info->AddNewTrack(Type::Face, track_id);
      }
    }
  }

  box_list = std::static_pointer_cast<BaseDataVector>(head_bbox);
  for (const auto &data : box_list->datas_) {
    auto box = std::static_pointer_cast<XStreamBBox>(data);
    if (box->state_ == xstream::DataState::VALID) {
      auto &track_id = box->value.id;
      auto iter = id_merge_info->GetHeadMap().find(track_id);
      if (iter == id_merge_info->GetHeadMap().end()) {
        id_merge_info->AddNewTrack(Type::Head, track_id);
      }
    }
  }
}

void HeadFaceStrategy::RemoveDisappearedTrack(
    const BaseDataPtr &disappearedFacePtr,
    const BaseDataPtr &disappearedHeadPtr,
    const BaseDataVectorPtr &DisappereadTrackIdPtr) {
  RemoveDisappearedMotId(Type::Face, disappearedFacePtr, DisappereadTrackIdPtr,
                         &id_relation_info_);
  RemoveDisappearedMotId(Type::Head, disappearedHeadPtr, DisappereadTrackIdPtr,
                         &id_relation_info_);
  for (auto &lost_id : conflict_ids_) {
    auto track_id = std::make_shared<XStreamUint32>();
    track_id->value = lost_id;
    track_id->state_ = xstream::DataState::DISAPPEARED;
    DisappereadTrackIdPtr->datas_.emplace_back(BaseDataPtr(track_id));
  }
  conflict_ids_.clear();
}

void HeadFaceStrategy::ProduceOutput(const BaseDataPtr &facePtr,
                                     const BaseDataPtr &headPtr,
                                     const BaseDataVectorPtr &faceResPtr,
                                     const BaseDataVectorPtr &headResPtr) {
  auto cur_face_id_set = GetIdSet(facePtr);
  auto cur_head_id_set = GetIdSet(headPtr);
  MergeFaceHeadTrackID(facePtr, headPtr, cur_face_id_set, cur_head_id_set,
                       &id_relation_info_);
  GetMainId(facePtr, id_relation_info_.GetFaceMap(), faceResPtr);
  GetMainId(headPtr, id_relation_info_.GetHeadMap(), headResPtr);
}
}  // namespace xstream
