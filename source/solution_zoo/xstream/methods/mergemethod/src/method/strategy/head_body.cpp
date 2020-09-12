/*
 * @Description: implement of data_type
 * @Author: peng02.li@horizon.ai
 * @Date: 2018-10-21 17:49:26
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-12-19 15:27:59
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */

#include "MergeMethod/strategy/head_body.h"

#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>

namespace xstream {

int HeadBodyParam::UpdateParameter(const JsonReaderPtr &reader) {
  LOGD << "HeadBodyParam update config: " << this;
  auto ret = MergeParam::UpdateParameter(reader);
  if (ret != kHorizonVisionSuccess) {
    return ret;
  } else {
    SET_SNAPSHOT_METHOD_PARAM(reader->GetRawJson(), Bool, use_kps);
    SET_SNAPSHOT_METHOD_PARAM(reader->GetRawJson(), Bool, double_thresh_flag);
    SET_SNAPSHOT_METHOD_PARAM(reader->GetRawJson(), Double, kps_cnt_threshold);
    SET_SNAPSHOT_METHOD_PARAM(reader->GetRawJson(), Double, conflict_threshold);
    SET_SNAPSHOT_METHOD_PARAM(reader->GetRawJson(), Double,
                              valid_kps_score_thresh);
    SET_SNAPSHOT_METHOD_PARAM(reader->GetRawJson(), Double, head_extend_ratio);
    LOGD << "use_kps: " << use_kps;
    LOGD << "double_thresh_flag: " << double_thresh_flag;
    LOGD << "kps_cnt_threshold: " << kps_cnt_threshold;
    LOGD << "conflict_threshold: " << conflict_threshold;
    LOGD << "valid_kps_score_thresh: " << valid_kps_score_thresh;
    LOGD << "head_extend_ratio: " << head_extend_ratio;
    return kHorizonVisionSuccess;
  }
}

std::vector<BaseDataPtr> HeadBodyStrategy::ProcessFrame(
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
      std::string content = param->Format();
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
  LOGD << "Head Face Mode, out size: " << out.size();
  return out;
}

void HeadBodyStrategy::Finalize() { LOGD << "HeadBodyStrategy Finalize"; }

int HeadBodyStrategy::UpdateParameter(const JsonReaderPtr &reader) {
  return (std::static_pointer_cast<HeadBodyParam>(merge_config_param_))
      ->UpdateParameter(reader);
}

std::shared_ptr<HeadBodyParam> HeadBodyStrategy::GetConfig() {
  auto config_ = std::static_pointer_cast<HeadBodyParam>(merge_config_param_);
  return config_;
}

int HeadBodyStrategy::Init(std::shared_ptr<MergeParam> config) {
  merge_config_param_ = config;
  return kHorizonVisionSuccess;
}

void HeadBodyStrategy::PassThroughSingleFrame(
    const std::vector<BaseDataPtr> &frame_input,
    std::vector<BaseDataPtr> *frame_output) {
  HOBOT_CHECK_GE(frame_input.size(), uint(5));
  // output is face box1
  frame_output->emplace_back(frame_input[0]);
  // output is head box2
  frame_output->emplace_back(frame_input[1]);
  // output is body box3
  frame_output->emplace_back(frame_input[2]);
  // output is disappeared_head_id
  frame_output->emplace_back(frame_input[4]);
  LOGD << "Process passthrough output";
}

void HeadBodyStrategy::RunSingleFrame(
    const std::vector<BaseDataPtr> &frame_input,
    std::vector<BaseDataPtr> *frame_output) {
  HOBOT_CHECK_GE(frame_input.size(), uint(5));
  auto config = GetConfig();
  const auto &face_box_list = frame_input[0];
  const auto &head_box_list = frame_input[1];
  const auto &body_box_list = frame_input[2];
  const auto &disappeared_face_id = frame_input[3];
  const auto &disappeared_head_id = frame_input[4];
  const auto &disappeared_body_id = frame_input[5];
  BaseDataPtr body_kps_list = nullptr;
  if (config->use_kps) {
    body_kps_list = frame_input[6];
    HOBOT_CHECK_EQ(body_kps_list->type_, "BaseDataVector");
  }

  HOBOT_CHECK_EQ(face_box_list->type_, "BaseDataVector");
  HOBOT_CHECK_EQ(head_box_list->type_, "BaseDataVector");
  HOBOT_CHECK_EQ(body_box_list->type_, "BaseDataVector");
  HOBOT_CHECK_EQ(disappeared_head_id->type_, "BaseDataVector");
  HOBOT_CHECK_EQ(disappeared_face_id->type_, "BaseDataVector");
  HOBOT_CHECK_EQ(disappeared_body_id->type_, "BaseDataVector");

  BaseDataVectorPtr face_box_res = std::make_shared<BaseDataVector>();
  BaseDataVectorPtr head_box_res = std::make_shared<BaseDataVector>();
  BaseDataVectorPtr body_box_res = std::make_shared<BaseDataVector>();
  BaseDataVectorPtr disappeared_track_id = std::make_shared<BaseDataVector>();

  ProduceOutput(face_box_list, head_box_list, body_box_list, body_kps_list,
                face_box_res, head_box_res, body_box_res);

  disappeared_track_id =
      std::static_pointer_cast<BaseDataVector>(disappeared_head_id);

  frame_output->emplace_back(BaseDataPtr(face_box_res));
  frame_output->emplace_back(BaseDataPtr(head_box_res));
  frame_output->emplace_back(BaseDataPtr(body_box_res));
  frame_output->emplace_back(BaseDataPtr(disappeared_track_id));
  LOGI << "Process HeadBodyStrategy output";
}

void HeadBodyStrategy::ProduceOutput(const BaseDataPtr &faceBoxPtr,
                                     const BaseDataPtr &headBoxPtr,
                                     const BaseDataPtr &bodyBoxPtr,
                                     const BaseDataPtr &bodyKpsPtr,
                                     const BaseDataVectorPtr &faceResPtr,
                                     const BaseDataVectorPtr &headResPtr,
                                     const BaseDataVectorPtr &bodyResPtr) {
  auto config = GetConfig();
  auto head_face_idx_pairs = GetHeadFacePairs(headBoxPtr, faceBoxPtr);
  std::vector<std::pair<int, int>> head_body_idx_pairs;
  if (config->use_kps) {
    head_body_idx_pairs =
        GetHeadBodyPairsWithKps(headBoxPtr, bodyBoxPtr, bodyKpsPtr);
  } else {
    head_body_idx_pairs = GetHeadBodyPairsWithoutKps(
        headBoxPtr, bodyBoxPtr, config->double_thresh_flag);
  }
  auto head_box_list = std::static_pointer_cast<BaseDataVector>(headBoxPtr);
  auto face_box_list = std::static_pointer_cast<BaseDataVector>(faceBoxPtr);
  auto body_box_list = std::static_pointer_cast<BaseDataVector>(bodyBoxPtr);
  LOGD << "face in size: " << face_box_list->datas_.size()
       << " head in size: " << head_box_list->datas_.size()
       << " body in size: " << body_box_list->datas_.size()
       << " head_face_idx_pairs size: " << head_face_idx_pairs.size()
       << " head_body_idx_pairs size: " << head_body_idx_pairs.size()
       << std::endl;

  // for (auto &face : face_box_list->datas_) {
  //   auto box = std::static_pointer_cast<XStreamBBox>(face);
  //   LOGD << "face box x1: " << box->value.x1 << " y1: " << box->value.y1
  //        << " x2: " << box->value.x2 << " y2: " << box->value.y2;
  // }
  // for (auto &head : head_box_list->datas_) {
  //   auto box = std::static_pointer_cast<XStreamBBox>(head);
  //   LOGD << "head box x1: " << box->value.x1 << " y1: " << box->value.y1
  //        << " x2: " << box->value.x2 << " y2: " << box->value.y2;
  // }
  // for (auto &body : body_box_list->datas_) {
  //   auto box = std::static_pointer_cast<XStreamBBox>(body);
  //   LOGD << "body box x1: " << box->value.x1 << " y1: " << box->value.y1
  //        << " x2: " << box->value.x2 << " y2: " << box->value.y2;
  // }

  std::unordered_map<int, int> face_head_idx_map;
  std::unordered_map<int, int> body_head_idx_map;
  Pairs2Map(head_face_idx_pairs, &face_head_idx_map);
  Pairs2Map(head_body_idx_pairs, &body_head_idx_map);
  for (size_t face_idx = 0; face_idx < face_box_list->datas_.size();
       ++face_idx) {
    auto face_box =
        std::static_pointer_cast<XStreamBBox>(face_box_list->datas_[face_idx]);
    auto bbox = std::make_shared<XStreamBBox>();
    bbox->value.x1 = face_box->value.x1;
    bbox->value.y1 = face_box->value.y1;
    bbox->value.x2 = face_box->value.x2;
    bbox->value.y2 = face_box->value.y2;
    bbox->value.score = face_box->value.score;

    if (face_head_idx_map.find(face_idx) == face_head_idx_map.end() ||
        xstream::DataState::VALID != face_box_list->datas_[face_idx]->state_) {
      bbox->value.id = -1;
      bbox->state_ = DataState::INVALID;
    } else {
      auto head_box = std::static_pointer_cast<XStreamBBox>(
          head_box_list->datas_[face_head_idx_map[face_idx]]);
      bbox->value.id = head_box->value.id;
      if (xstream::DataState::VALID !=
          head_box_list->datas_[face_head_idx_map[face_idx]]->state_)
        bbox->state_ = xstream::DataState::INVALID;
    }
    faceResPtr->datas_.emplace_back(BaseDataPtr(bbox));
  }

  for (size_t body_idx = 0; body_idx < body_box_list->datas_.size();
       ++body_idx) {
    auto body_box =
        std::static_pointer_cast<XStreamBBox>(body_box_list->datas_[body_idx]);
    auto bbox = std::make_shared<XStreamBBox>();
    bbox->value.x1 = body_box->value.x1;
    bbox->value.y1 = body_box->value.y1;
    bbox->value.x2 = body_box->value.x2;
    bbox->value.y2 = body_box->value.y2;
    bbox->value.score = body_box->value.score;
    if (body_head_idx_map.find(body_idx) == body_head_idx_map.end() ||
        xstream::DataState::VALID != body_box_list->datas_[body_idx]->state_) {
      bbox->value.id = -1;
      bbox->state_ = DataState::INVALID;
    } else {
      auto head_box = std::static_pointer_cast<XStreamBBox>(
          head_box_list->datas_[body_head_idx_map[body_idx]]);
      bbox->value.id = head_box->value.id;
      if (xstream::DataState::VALID !=
          head_box_list->datas_[body_head_idx_map[body_idx]]->state_)
        bbox->state_ = xstream::DataState::INVALID;
    }
    bodyResPtr->datas_.emplace_back(BaseDataPtr(bbox));
  }

  for (size_t head_idx = 0; head_idx < head_box_list->datas_.size();
       ++head_idx) {
    auto head_box =
        std::static_pointer_cast<XStreamBBox>(head_box_list->datas_[head_idx]);
    auto bbox = std::make_shared<XStreamBBox>();
    bbox->value.x1 = head_box->value.x1;
    bbox->value.y1 = head_box->value.y1;
    bbox->value.x2 = head_box->value.x2;
    bbox->value.y2 = head_box->value.y2;
    bbox->value.score = head_box->value.score;
    bbox->value.id = head_box->value.id;
    if (xstream::DataState::VALID != head_box_list->datas_[head_idx]->state_)
      bbox->state_ = xstream::DataState::INVALID;
    headResPtr->datas_.emplace_back(BaseDataPtr(bbox));
  }

  LOGD << "face out size: " << faceResPtr->datas_.size()
       << " head out size: " << headResPtr->datas_.size()
       << " body out size: " << bodyResPtr->datas_.size() << std::endl;
}

void HeadBodyStrategy::Pairs2Map(const std::vector<std::pair<int, int>> &pairs,
                                 std::unordered_map<int, int> *index_map) {
  index_map->clear();
  int idx1, idx2;
  for (auto &pair_item : pairs) {
    idx1 = pair_item.second;
    idx2 = pair_item.first;
    index_map->insert(std::make_pair(idx1, idx2));
  }
}

std::vector<std::pair<int, int>> HeadBodyStrategy::GetHeadFacePairs(
    const BaseDataPtr &head_box_ptr, const BaseDataPtr &face_box_ptr) {
  std::vector<std::pair<int, int>> pairs;
  auto config = GetConfig();
  auto head_box_list = std::static_pointer_cast<BaseDataVector>(head_box_ptr);
  auto face_box_list = std::static_pointer_cast<BaseDataVector>(face_box_ptr);
  int n_head = head_box_list->datas_.size();
  int n_face = face_box_list->datas_.size();

  float thresh = config->match_threshold;
  if (n_head < 1 || n_face < 1) {
    return pairs;
  }
  std::vector<float> score_mat(n_head * n_face);
  for (int head_track_i = 0; head_track_i < n_head; ++head_track_i) {
    auto head_data = head_box_list->datas_[head_track_i];
    auto head_box = std::static_pointer_cast<XStreamBBox>(head_data);
    for (int face_track_i = 0; face_track_i < n_face; ++face_track_i) {
      auto face_data = face_box_list->datas_[face_track_i];
      auto face_box = std::static_pointer_cast<XStreamBBox>(face_data);
      float intersect_area = intersection(head_box->value, face_box->value);
      float face_area =
          (face_box->value.Width() + 1) * (face_box->value.Height() + 1);
      score_mat[head_track_i * n_face + face_track_i] =
          intersect_area / face_area;
    }
  }
  pairs = GreedySearch(&score_mat, n_face, n_head, thresh);
  LOGD << "Pairs size: " << pairs.size();
  return pairs;
}

std::vector<std::pair<int, int>> HeadBodyStrategy::GetHeadBodyPairsWithoutKps(
    const BaseDataPtr &head_box_ptr, const BaseDataPtr &body_box_ptr,
    bool double_thresh_flag) {
  std::vector<std::pair<int, int>> pairs;
  auto config = GetConfig();
  auto head_box_list = std::static_pointer_cast<BaseDataVector>(head_box_ptr);
  auto body_box_list = std::static_pointer_cast<BaseDataVector>(body_box_ptr);
  int n_head = head_box_list->datas_.size();
  int n_body = body_box_list->datas_.size();

  float thresh = config->match_threshold;
  if (n_head < 1 || n_body < 1) {
    return pairs;
  }
  std::vector<float> score_mat(n_head * n_body);
  for (int head_track_i = 0; head_track_i < n_head; ++head_track_i) {
    auto head_data = head_box_list->datas_[head_track_i];
    auto head_box = std::static_pointer_cast<XStreamBBox>(head_data);
    for (int body_track_i = 0; body_track_i < n_body; ++body_track_i) {
      auto body_data = body_box_list->datas_[body_track_i];
      auto body_box = std::static_pointer_cast<XStreamBBox>(body_data);
      float intersect_area = intersection(body_box->value, head_box->value);
      hobot::vision::BBox one_third_upper_body(
          body_box->value.x1, body_box->value.y1, body_box->value.x2,
          body_box->value.y1 + 0.3 * body_box->value.Height());
      float upper_intersection_area =
          intersection(one_third_upper_body, head_box->value);
      float height_ratio = head_box->value.Height() / body_box->value.Height();
      if (intersect_area < 1e-4 || upper_intersection_area < 1e-4 ||
          height_ratio < 0.1) {
        continue;
      }
      float head_area = head_box->value.Height() * head_box->value.Width();
      float y_score =
          (body_box->value.y2 - head_box->value.y1) / body_box->value.Height();
      float ratio = intersect_area / head_area;
      if (y_score < thresh) {
        y_score = 0;
      }
      score_mat[head_track_i * n_body + body_track_i] =
          0.7 * ratio + 0.3 * y_score;
      if (ratio < thresh) {
        score_mat[head_track_i * n_body + body_track_i] = 0;
      }
    }
  }
  if (double_thresh_flag) {
    int n_match = 0;
    float conflict_thresh = config->conflict_threshold;
    for (int j = 0; j < n_body; ++j) {
      for (int i = 0; i < n_head; ++i) {
        if (score_mat[i * n_body + j] > conflict_thresh) {
          n_match += 1;
        }
      }
      if (n_match > 1) {
        for (int i = 0; i < n_head; ++i) {
          score_mat[i * n_body + j] = 0;
        }
      }
    }
  }
  pairs = GreedySearch(&score_mat, n_body, n_head, thresh);
  LOGD << "Pairs size: " << pairs.size();
  return pairs;
}

// 使用人头与KPS的策略
std::vector<std::pair<int, int>> HeadBodyStrategy::GetHeadBodyPairsWithKps(
    const BaseDataPtr &head_box_ptr, const BaseDataPtr &body_box_ptr,
    const BaseDataPtr &body_kps_ptr) {
  std::vector<std::pair<int, int>> pairs;
  auto config = GetConfig();
  auto head_box_list = std::static_pointer_cast<BaseDataVector>(head_box_ptr);
  auto body_box_list = std::static_pointer_cast<BaseDataVector>(body_box_ptr);
  auto body_kps_list = std::static_pointer_cast<BaseDataVector>(body_kps_ptr);

  int n_head = head_box_list->datas_.size();
  int n_body = body_box_list->datas_.size();

  float valid_kps_score_thresh = config->valid_kps_score_thresh;
  float kps_cnt_thresh = config->kps_cnt_threshold;
  float head_extend_ratio = config->head_extend_ratio;

  if (n_head < 1 || n_body < 1) {
    return pairs;
  }
  std::vector<float> score_mat(n_head * n_body);
  for (int head_track_i = 0; head_track_i < n_head; ++head_track_i) {
    auto head_data = head_box_list->datas_[head_track_i];
    auto head_box = std::static_pointer_cast<XStreamBBox>(head_data);
    for (int body_track_i = 0; body_track_i < n_body; ++body_track_i) {
      auto body_data = body_kps_list->datas_[body_track_i];
      auto body_kps = std::static_pointer_cast<XStreamKps>(body_data);
      // expand head box
      hobot::vision::BBox expand_head_box(
          head_box->value.x1 - head_extend_ratio * head_box->value.Width(),
          head_box->value.y1 - head_extend_ratio * head_box->value.Height(),
          head_box->value.x2 + head_extend_ratio * head_box->value.Width(),
          head_box->value.y2 + head_extend_ratio * head_box->value.Height());
      int n_in_box = 0;
      for (size_t kps_i = 0; kps_i < 17; kps_i++) {
        auto &point = body_kps->value.values[kps_i];
        if (point.score >= valid_kps_score_thresh) {
          if (point.x >= expand_head_box.x1 && point.x <= expand_head_box.x2 &&
              point.y >= expand_head_box.y1 && point.y <= expand_head_box.y2) {
            n_in_box += 1;
          }
        }
      }
      score_mat[head_track_i * n_body + body_track_i] = n_in_box;
    }
  }
  pairs = GreedySearch(&score_mat, n_body, n_head, kps_cnt_thresh);
  LOGD << "Pairs size: " << pairs.size();
  return pairs;
}

// 贪婪匹配
std::vector<std::pair<int, int>> HeadBodyStrategy::GreedySearch(
    std::vector<float> *score_mat, int widht, int height, float thresh) {
  assert(static_cast<int>(score_mat->size()) == widht * height);
  std::vector<std::pair<int, int>> pairs;
  std::vector<float>::iterator pos;
  int pos_i, pos_j, index;
  pos = std::max_element(score_mat->begin(), score_mat->end());
  index = pos - score_mat->begin();
  pos_i = index / widht;
  pos_j = index % widht;
  while (*pos >= thresh) {
    pairs.push_back(std::make_pair(pos_i, pos_j));
    for (int i = 0; i < widht; ++i) {
      // row
      score_mat->at(pos_i * widht + i) = 0;
    }
    for (int j = 0; j < height; ++j) {
      // col
      score_mat->at(pos_j + j * widht) = 0;
    }
    pos = std::max_element(score_mat->begin(), score_mat->end());
    index = pos - score_mat->begin();
    pos_i = index / widht;
    pos_j = index % widht;
  }
  return pairs;
}

}  // namespace xstream
