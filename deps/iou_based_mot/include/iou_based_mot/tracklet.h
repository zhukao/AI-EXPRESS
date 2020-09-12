//
// Created by kai01.han on 6/1/19.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#ifndef IOU_BASED_MOT_TRACKLET_H
#define IOU_BASED_MOT_TRACKLET_H

#include <memory>
#include "iou_based_mot/target.h"
#include "iou_based_mot/kalmanBoxTracker.h"
#include "iou_based_mot/config.h"

namespace hobot {
namespace iou_mot {
class TrackLet {
 public:
  enum TrackLetState {
   Tentative = 0,
   Confirmed = 1,
   Invisible = 2,
   Deleted = 3
  };

  TrackLet(const int & track_id,
          const sp_Target &target,
          const long long &frame_id,
          const time_t &time_stamp);

  ~TrackLet();

  int track_id = -1;
  sp_Target target;
  std::map<time_t, sp_Target> target_map;
  std::vector<int> frame_id_list;
  std::vector<time_t> time_stamp_list;
  time_t last_time_stamp;
  TrackLetState state = Tentative;
  int missed_times = 0;
  double dist = 0;
  spKalmanBoxTracker kf_obj = nullptr;
  std::string matched_state = "NC";
  sp_BBox kf_last;

  void add(const sp_Target &target,
          const long long & frame_id,
          const time_t & time_stamp,
          double dist = 0.,
          std::string matched_state = "NC");
  void MarkMissed(const time_t &time_stamp_now);
  sp_Target GetTargetOfTimeStamp(const time_t &time_stamp, bool &flag);
};
typedef std::shared_ptr<TrackLet> sp_TrackLet;

} // namespace iou_mot
} // namespace hobot


#endif //IOU_BASED_MOT_TRACKLET_H
