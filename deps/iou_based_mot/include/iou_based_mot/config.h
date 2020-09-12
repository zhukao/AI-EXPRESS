//
// Created by kai01.han on 6/3/19.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#ifndef IOU_BASED_MOT_CONFIG_H_
#define IOU_BASED_MOT_CONFIG_H_

#include "hobot/hobot.h"
#include "hobotlog/hobotlog.hpp"

namespace hobot {
namespace iou_mot {

struct TrackParam {
  std::string match_type;
  bool use_kalman_filter;
  int missing_time_thres;
  int remove_invisible_track_ms;
  int remove_obsolete_track;
  float iou_thres;
  float euclidean_thres;
  int use_location_gain;
  int max_trajectory_number;
};

struct DetectionParam {
  float min_score;
  float ignore_overlap_thres;
};

class ConfigMessage : public hobot::Message {
 public:
  ConfigMessage() {}
  ~ConfigMessage() {}

  inline static ConfigMessage *instance() {
    static ConfigMessage *config = nullptr;
    if (nullptr == config) {
      config = new ConfigMessage();
    }
    return config;
  }

  void SetConfig(std::string config_path);

  TrackParam track_param;
  DetectionParam detection_param;
};

typedef std::shared_ptr<ConfigMessage> spConfigMessage;

}  // namespace iou_mot
}  // namespace hobot

#endif  // IOU_BASED_MOT_CONFIG_H_
