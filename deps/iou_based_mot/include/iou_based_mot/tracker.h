//
// Created by kai01.han on 6/1/19.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#ifndef IOU_BASED_MOT_TRACKER_H
#define IOU_BASED_MOT_TRACKER_H

#include <memory>
#include <cstdlib>
#include <cassert>
#include "iou_based_mot/target.h"
#include "iou_based_mot/tracklet.h"

namespace hobot {
namespace iou_mot {
const double MAX_DISTANCE = 10000.0;

class Tracker {
public:
  Tracker() {
    this->curr_time_stamp = -1;
    this->begin_time_stamp = -1;
    this->last_output_time_stamp = -1;
  };
  explicit Tracker(time_t time_stamp) {
    this->curr_time_stamp = time_stamp;
    this->begin_time_stamp = time_stamp;
    this->last_output_time_stamp = time_stamp;
  }

  ~Tracker() {
    tracklet_list_.clear();
    target_list.clear();
  }

  const std::vector<sp_TrackLet> tracklet_list() {
    return tracklet_list_;
  }

  void TrackPro(const std::vector<sp_BBox> &body_bbox_list,
                const time_t &time_stamp,
                const int &img_width,
                const int &img_height);

  void BuildTargetList(const std::vector<sp_BBox> &detected_boxes);
  inline void OutputBox(const BBox_s &box) {
    std::cout << box.x1 << " " << box.y1 << " " << box.x2 <<
    " " << box.y2 << std::endl;
  }
private:
  void MatchTrack2Target(std::vector<std::tuple<int, int, double,
          std::string>> &matches, std::vector<int> &unmatches_tracks,
          std::vector<int> &unmatches_targets);
  void Track();
  static void CheckInput(const std::vector<sp_BBox> &body_bbox_list,
                         const time_t &time_stamp,
                         const int img_width,
                         const int img_height){
    assert(time_stamp >= 0);
    assert(img_height > 0 && img_width > 0);
  }

  std::vector<sp_TrackLet> tracklet_list_;
  std::vector<sp_Target> target_list;
  bool is_ready_to_output = false;
  long long frame_counter = 0;
  int track_id_counter = 0;
  time_t curr_time_stamp;
  time_t begin_time_stamp;
  time_t last_output_time_stamp;

  std::vector<int> id_2_ori_id;
  int img_width = 1920;
  int img_height =1080;

  Config config;
};

} // namespace iou_mot
} // namespace hobot

#endif //IOU_BASED_MOT_TRACKER_H
