/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @brief     vote_method
 * @author    tangji.sun
 * @date      2020.2.6
 */

#include "vote_method/vote_method.h"

#include <assert.h>
#include <algorithm>
#include <fstream>
#include <memory>
#include <mutex>
#include <numeric>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>

#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_data.h"
#include "json/json.h"

using hobot::vision::AntiSpoofing;
using hobot::vision::BBox;

const int VEHICLE_TYPE_COLOR = 0;
const int PLATE_COLOR = 1;
const int LIVING_BEHAVIOR = 2;
const int AGE = 3;
const int GENDER = 4;
const int TIMEINTERVAL = 5;
const int InvalidType = -1;

const int PositiveDet = 1;
const int NegativeDet = 0;
const int InvalidDet = -1;

namespace xstream {

int VoteMethod::Init(const std::string &config_file_path) {
  LOGI << "Init " << config_file_path << std::endl;
  std::ifstream config_ifs(config_file_path);
  if (!config_ifs.good()) {
    LOGF << "open config file failed.";
  }
  Json::Value config_jv;
  config_ifs >> config_jv;
  if (config_jv.isMember("type") && config_jv["type"].isString()) {
    auto type = config_jv["type"].asString();
    LOGI << "type:" << type;
    if (type == "vehicle") {
      type_ = VEHICLE_TYPE_COLOR;
    } else if (type == "plate_color") {
      type_ = PLATE_COLOR;
    } else if (type == "age") {
      type_ = AGE;
    } else if (type == "gender") {
      type_ = GENDER;
    } else if (type == "living" || type == "behavior") {
      type_ = LIVING_BEHAVIOR;
      positive_voting_threshold_ =
          config_jv[type]["positive_voting_threshold"].asFloat();
      negative_voting_threshold_ =
          config_jv[type]["negative_voting_threshold"].asFloat();
    } else if (type == "time_interval") {
      type_ = TIMEINTERVAL;
      time_interval_ = config_jv["time_interval_val"].asFloat();
    } else {
      LOGF << "config file error.";
    }
    if (type != "time_interval") {
      max_slide_window_size_ = config_jv[type]["max_slide_window_size"].asInt();
    }
  } else {
    // default type
    type_ = VEHICLE_TYPE_COLOR;
    max_slide_window_size_ = 25;
  }

  if (type_ != TIMEINTERVAL) {
    HOBOT_CHECK(max_slide_window_size_ > 0);
  }
  method_param_ = nullptr;

  return 0;
}

std::vector<std::vector<BaseDataPtr>> VoteMethod::DoProcess(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<xstream::InputParamPtr> &param) {
  LOGI << "Run VoteMethod";
  LOGD << "input's size: " << input.size();

  std::vector<std::vector<BaseDataPtr>> output;
  output.resize(input.size());

  for (size_t i = 0; i < input.size(); ++i) {
    const auto &frame_input = input[i];
    auto &frame_output = output[i];
    RunSingleFrame(frame_input, frame_output);
  }
  return output;
}

void VoteMethod::AdjustQueue(const XStreamData<int> &vote_info,
                             uint32_t track_id,
                             float score,
                             float timestamp) {
  // TODO(shiyu.fu): review timeinterval
  if (type_ == TIMEINTERVAL) {
    if (vote_info.value != InvalidType) {
      auto vote_info_iter = slide_window_map.find(track_id);
      auto timestamp_iter = timestamp_map_.find(track_id);
      if (vote_info_iter == slide_window_map.end() &&
          timestamp_iter == timestamp_map_.end()) {
        slide_window_map[track_id].push_back(vote_info);
        timestamp_map_[track_id].push_back(timestamp);
      } else {
        auto &vote_info_queue = slide_window_map[track_id];
        auto &timestamp_queue = timestamp_map_[track_id];
        vote_info_queue.push_back(vote_info);
        timestamp_queue.push_back(timestamp);
        float front_timestamp = timestamp_queue.front();
        while (timestamp - front_timestamp > time_interval_) {
          timestamp_queue.pop_front();
          front_timestamp = timestamp_queue.front();
        }
        while (vote_info_queue.size() > timestamp_queue.size()) {
          vote_info_queue.pop_front();
        }
        HOBOT_CHECK(slide_window_map[track_id].size() ==
                    timestamp_map_[track_id].size())
                    << "#vote_info & #timestamp mismatch";
      }
    }
  } else if (type_ == LIVING_BEHAVIOR || type_ == GENDER ||
      vote_info.value != InvalidType) {
    auto iter = slide_window_map.find(track_id);
    if (iter == slide_window_map.end()) {
      slide_window_map[track_id].push_back(vote_info);
      score_[track_id] = score;
    } else {
      int queue_size = slide_window_map[track_id].size();
      if (queue_size < max_slide_window_size_) {
        slide_window_map[track_id].push_back(vote_info);
      } else if (queue_size == max_slide_window_size_) {
        slide_window_map[track_id].pop_front();
        assert(slide_window_map[track_id].size()
          == static_cast<std::size_t>(queue_size - 1));
        slide_window_map[track_id].push_back(vote_info);
      } else {
        HOBOT_CHECK(0) << "impossible.";
      }
      if (score > score_[track_id]) {
        score_[track_id] = score;
      }
    }
  }
}

void VoteMethod::Vote(std::shared_ptr<XStreamData<int>> &vote_info_ptr,
                      uint32_t track_id) {
  auto iter = slide_window_map.find(track_id);
  if (iter == slide_window_map.end()) {
    vote_info_ptr->value = InvalidType;
  } else {
    auto &slide_window = slide_window_map[track_id];
    int window_size = slide_window.size();
    if (type_ == LIVING_BEHAVIOR) {
      if (window_size < max_slide_window_size_) {
        vote_info_ptr->value = InvalidDet;
      } else if (window_size == max_slide_window_size_) {
        int pos_num = 0;
        int neg_num = 0;
        int invalid_num = 0;
        // 计算活体、非活体、无效信息在滑动窗口中的数量
        for (const auto &det_ret : slide_window) {
          if (det_ret.value == PositiveDet) {
            ++pos_num;
          } else if (det_ret.value == NegativeDet) {
            ++neg_num;
          } else if (det_ret.value == InvalidDet) {
            ++invalid_num;
          } else {
            HOBOT_CHECK(0) << "impossible.";
          }
        }
        float pos_voting_ratio = static_cast<float>(pos_num) /
                                 static_cast<float>(max_slide_window_size_);
        float neg_voting_ratio = static_cast<float>(neg_num) /
                                 static_cast<float>(max_slide_window_size_);
        if (pos_voting_ratio > positive_voting_threshold_) {
          vote_info_ptr->value = PositiveDet;
        } else if (neg_voting_ratio > negative_voting_threshold_) {
          vote_info_ptr->value = NegativeDet;
        } else {
          vote_info_ptr->value = InvalidDet;
        }
      } else {
        HOBOT_CHECK(0) << "impossible.";
      }

    } else {
      if (window_size <= max_slide_window_size_ || type_ == TIMEINTERVAL) {
        std::unordered_map<int, uint32_t> vote;  // type, count
        for (const auto &info : slide_window) {
          if (vote.find(info.value) == vote.end()) {
            vote[info.value] = 1;
          } else {
            auto value = vote[info.value];
            vote[info.value] = ++value;
          }
        }

        uint32_t max = 0;
        int type = InvalidType;
        LOGD << "start vote";
        for (auto vote_iter = vote.begin(); vote_iter != vote.end();
             vote_iter++) {
          if (max <= vote_iter->second) {
            max = vote_iter->second;
            type = vote_iter->first;
          }
        }
        vote_info_ptr->value = type;
        LOGD << "end vote:" << type;

      } else {
        HOBOT_CHECK(0) << "impossible.";
      }
    }
  }
}

void VoteMethod::RunSingleFrame(const std::vector<BaseDataPtr> &frame_input,
                                std::vector<BaseDataPtr> &frame_output) {
  HOBOT_CHECK(frame_input.size() == 3);  // BBox, disappeard track_id and type.
  auto out_track_ids = std::make_shared<BaseDataVector>();
  auto out_infos = std::make_shared<BaseDataVector>();
  if (type_ == LIVING_BEHAVIOR) {
    frame_output.resize(2);  // track_id, and liveness.
    frame_output[0] = out_track_ids;
    frame_output[1] = out_infos;
  } else {
    frame_output.resize(1);  // type, age, gender
    frame_output[0] = out_infos;
  }
  LOGI << "type" << type_;

  std::vector<BaseDataPtr> boxes;
  if (frame_input[0]) {
    boxes = std::static_pointer_cast<BaseDataVector>(frame_input[0])->datas_;
  }

  std::vector<BaseDataPtr> disappeared_track_ids;
  if (frame_input[1]) {
    disappeared_track_ids =
        std::static_pointer_cast<BaseDataVector>(frame_input[1])->datas_;
  }

  std::vector<BaseDataPtr> vote_infos;
  if (frame_input[2]) {
    vote_infos =
        std::static_pointer_cast<BaseDataVector>(frame_input[2])->datas_;
  }
  LOGD << "box num: " << boxes.size();
  LOGD << "vote info: " << vote_infos.size();
  HOBOT_CHECK(boxes.size() == vote_infos.size());

  for (size_t i = 0; i < boxes.size(); ++i) {
    const auto &box = std::static_pointer_cast<XStreamData<BBox>>(
                      boxes[i])->value;
    // assert(box.id >= 0);
    if (boxes[i]->state_ != xstream::DataState::VALID || box.id < 0) {
      if (type_ == VEHICLE_TYPE_COLOR) {
        std::shared_ptr<XStreamData<int>> invalid_vote(new XStreamData<int>());
        invalid_vote->value = -1;
        invalid_vote->state_ = DataState::INVALID;
        out_infos->datas_.push_back(invalid_vote);
      } else if (type_ == PLATE_COLOR || LIVING_BEHAVIOR) {
        auto invalid_vote = std::make_shared<
            XStreamData<hobot::vision::Attribute<int>>>();
        invalid_vote->value.value = -1;
        invalid_vote->value.score = -1;
        invalid_vote->state_ = DataState::INVALID;
        out_infos->datas_.push_back(invalid_vote);
        if (type_ == LIVING_BEHAVIOR) {
          auto invalid_track = std::make_shared<XStreamData<int>>();
          invalid_track->value = box.id;
          invalid_track->state_ = DataState::INVALID;
          out_track_ids->datas_.push_back(invalid_track);
        }
      } else if (type_ == AGE) {
        auto invalid_vote = std::make_shared<
            XStreamData<hobot::vision::Age>>();
        invalid_vote->state_ = DataState::INVALID;
        out_infos->datas_.push_back(invalid_vote);
      } else if (type_ == GENDER) {
        auto invalid_vote = std::make_shared<
            XStreamData<hobot::vision::Gender>>();
        invalid_vote->state_ = DataState::INVALID;
        out_infos->datas_.push_back(invalid_vote);
      }
      continue;
    }
    uint32_t track_id = static_cast<uint32_t>(box.id);
    XStreamData<int> vote_info;
    float score = 0.0;
    int info = -1;
    if (vote_infos[i]->state_ == DataState::VALID) {
      if (type_ == VEHICLE_TYPE_COLOR || type_ == TIMEINTERVAL) {
        info = std::static_pointer_cast<XStreamData<int>>(vote_infos[i])->value;
      } else if (type_ == PLATE_COLOR) {
        info =
          std::static_pointer_cast<XStreamData<hobot::vision::Attribute<int>>>(
              vote_infos[i])->value.value;
        score =
          std::static_pointer_cast<XStreamData<hobot::vision::Attribute<int>>>(
              vote_infos[i])->value.score;
        LOGD << "track_id:" << track_id << " info: " << info;
      } else if (type_ == LIVING_BEHAVIOR) {
        info =
          std::static_pointer_cast<XStreamData<hobot::vision::Attribute<int>>>(
              vote_infos[i])->value.value;
        info = info < 0 ? -1 : info;
        score =
          std::static_pointer_cast<XStreamData<hobot::vision::Attribute<int>>>(
              vote_infos[i])->value.score;
      } else if (type_ == AGE) {
        info = std::static_pointer_cast<XStreamData<hobot::vision::Age>>(
            vote_infos[i])->value.value;  // age_class: 0~7
      } else if (type_ == GENDER) {
        info = std::static_pointer_cast<XStreamData<hobot::vision::Gender>>(
            vote_infos[i])->value.value;  // 1 或 -1
      }

      LOGI << "info:" << info;
      auto vote_info_iter = slide_window_map.find(track_id);
      // if invaild use prev frame info
      if (type_ != LIVING_BEHAVIOR && type_ != GENDER &&
          type_ != TIMEINTERVAL && info == InvalidType) {
        if (vote_info_iter == slide_window_map.end()) {
          vote_info.value = InvalidType;
        } else {
          auto &vote_queue = vote_info_iter->second;
          if (vote_queue.empty()) {
            vote_info.value = InvalidType;
          } else {
            vote_info.value = vote_queue.back().value;
          }
        }
      } else {
        vote_info.value = info;
      }
    } else {
      LOGD << "invalid";
    }

    auto cur_microsec = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
    float timestamp = cur_microsec / 1000000.0;
    LOGD << "befor vote, value: " << vote_info.value;

    // adjust queue
    AdjustQueue(vote_info, track_id, score, timestamp);
    // vote
    std::shared_ptr<XStreamData<int>> vote_info_ptr(new XStreamData<int>());
    Vote(vote_info_ptr, track_id);
    // output
    if (type_ == VEHICLE_TYPE_COLOR || type_ == TIMEINTERVAL) {
      out_infos->datas_.push_back(vote_info_ptr);
    } else if (type_ == PLATE_COLOR) {
      std::shared_ptr<XStreamData<hobot::vision::Attribute<int>>> dataPtr(
          new XStreamData<hobot::vision::Attribute<int>>());
      dataPtr->value.value = vote_info_ptr->value;
      dataPtr->value.score = score_[track_id];
      out_infos->datas_.push_back(dataPtr);
    } else if (type_ == LIVING_BEHAVIOR) {
      std::shared_ptr<XStreamData<hobot::vision::Attribute<int>>> det_ret_ptr(
          new XStreamData<hobot::vision::Attribute<int>>());
      det_ret_ptr->value.value = vote_info_ptr->value;
      det_ret_ptr->value.score = score_[track_id];
      out_infos->datas_.push_back(det_ret_ptr);
      std::shared_ptr<XStreamData<uint32_t>> track_id_ptr(
          new XStreamData<uint32_t>());
      track_id_ptr->value = track_id;
      out_track_ids->datas_.push_back(track_id_ptr);
    } else if (type_ == AGE) {
      std::shared_ptr<XStreamData<hobot::vision::Age>> dataPtr(
          new XStreamData<hobot::vision::Age>());
      // TODO(zhe.sun) score
      dataPtr->value.value = vote_info_ptr->value;
      const std::vector<int> g_age_range = {1,  6,  7,  12, 13, 18, 19, 28,
                                      29, 35, 36, 45, 46, 55, 56, 100};
      dataPtr->value.min = g_age_range[vote_info_ptr->value * 2];
      dataPtr->value.max = g_age_range[vote_info_ptr->value * 2 + 1];
      out_infos->datas_.push_back(dataPtr);
    } else if (type_ == GENDER) {
      std::shared_ptr<XStreamData<hobot::vision::Gender>> dataPtr(
          new XStreamData<hobot::vision::Gender>());
      // TODO(zhe.sun) score
      dataPtr->value.value = vote_info_ptr->value;
      out_infos->datas_.push_back(dataPtr);
    }

    if (vote_info_ptr->value == InvalidType && type_ != GENDER) {
      out_infos->datas_.back()->state_ = DataState::INVALID;
    }
  }

  for (const auto &data : disappeared_track_ids) {
    // LOGD << "disappeared_track_ids loops";
    auto disappeared_track_id =
        std::static_pointer_cast<XStreamData<uint32_t>>(data)->value;
    auto iter = slide_window_map.find(disappeared_track_id);
    if (iter != slide_window_map.end()) {
      slide_window_map.erase(iter);
    }
    auto score_iter = score_.find(disappeared_track_id);
    if (score_iter != score_.end()) {
      score_.erase(score_iter);
    }
    auto timestamp_iter = timestamp_map_.find(disappeared_track_id);
    if (timestamp_iter != timestamp_map_.end()) {
      timestamp_map_.erase(timestamp_iter);
    }
  }

  // LOGD << "track_id2slide_window's size: " << slide_window_map.size();
}

}  // namespace xstream
