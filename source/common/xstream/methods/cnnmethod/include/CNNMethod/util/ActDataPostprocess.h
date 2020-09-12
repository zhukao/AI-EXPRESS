//
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
// Created by shiyu.fu on 08/27/20.
//

#ifndef CNNMETHOD_UTIL_ACTDATAPOSTPROCESS_H_
#define CNNMETHOD_UTIL_ACTDATAPOSTPROCESS_H_

#include <vector>
#include <deque>
#include <memory>
#include <unordered_map>
#include <mutex>
#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_data.h"
#include "xproto/utils/singleton.h"

using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::XStreamData;
using hobot::vision::Attribute;

class CachedScoreEntry {
 public:
  explicit CachedScoreEntry(float timestamp) : timestamp_(timestamp) {}
  void Clean() { score_.clear(); }
  std::vector<float> score_;
  float timestamp_;
};

class ActDataPostprocess : public hobot::CSingleton<ActDataPostprocess> {
 public:
  int Init(float window_size, int score_size) {
    cached_scores_map_.clear();
    window_size_ = window_size;
    score_size_ = score_size;
    return 0;
  }

  std::vector<float> GetCachedAvgScore(
    float timestamp, int track_id, std::vector<float> cur_score) {
    std::unique_lock<std::mutex> lock(map_mutex_);
    HOBOT_CHECK(cur_score.size() == static_cast<uint32_t>(score_size_))
      << "#scores mismatch!";
    // current score entry
    CachedScoreEntry cur_entry(timestamp);
    cur_entry.score_.assign(cur_score.begin(), cur_score.end());
    // get data for current track id
    auto iter = cached_scores_map_.find(track_id);
    if (iter == cached_scores_map_.end()) {
      std::deque<CachedScoreEntry> cached_scores;
      cached_scores.push_back(cur_entry);
      cached_scores_map_[track_id] = cached_scores;
      return cur_score;
    } else {
      auto cached_scores = iter->second;
      cached_scores.push_back(cur_entry);
      auto front_timestamp = cached_scores.front().timestamp_;
      while (timestamp - front_timestamp > window_size_) {
        cached_scores.pop_front();
        front_timestamp = cached_scores.front().timestamp_;
      }
      auto avg_score = CalcAvg(cached_scores);
      HOBOT_CHECK(avg_score.size() == static_cast<uint32_t>(score_size_))
        << "#scores mismatch!";
      return avg_score;
    }
  }

  void Clean(std::shared_ptr<BaseDataVector> disappeared_track_ids) {
    std::unique_lock<std::mutex> lock(map_mutex_);
    for (size_t i = 0; i < disappeared_track_ids->datas_.size(); ++i) {
      auto disappeared_track_id =
          std::static_pointer_cast<XStreamData<uint32_t>>(
                  disappeared_track_ids->datas_[i])->value;
      if (cached_scores_map_.find(disappeared_track_id) !=
          cached_scores_map_.end()) {
        cached_scores_map_[disappeared_track_id].clear();
        cached_scores_map_.erase(disappeared_track_id);
      }
    }
  }

 private:
  std::vector<float> CalcAvg(std::deque<CachedScoreEntry> data) {
    std::vector<float> avg_score;
    for (int score_idx = 0; score_idx < score_size_; ++score_idx) {
      float sum = 0;
      for (size_t data_idx = 0; data_idx < data.size(); ++data_idx) {
        sum += data[data_idx].score_[score_idx];
      }
      float avg = sum / static_cast<float>(data.size());
      avg_score.push_back(avg);
    }
    HOBOT_CHECK(avg_score.size() == static_cast<uint32_t>(score_size_))
      << "#score mismatch";
    return avg_score;
  }

 private:
  std::unordered_map<int, std::deque<CachedScoreEntry>> cached_scores_map_;
  float window_size_;
  int score_size_;
  std::mutex map_mutex_;
};

#endif  // CNNMETHOD_UTIL_ACTDATAPOSTPROCESS_H_
