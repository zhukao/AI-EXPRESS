/*************************************************************************************************
* Description:  Kalman Box Tracker based on kalmanfilter class.
* Author:    Jiang Lu (jianglu16@outlook.com).
* Date:      2017.09.08.
*                  All Right Reserved.
*  Copyright (c) 2019 Horizon Robotics. All rights reserved.
**************************************************************************************************/
#ifndef IOU_BASED_MOT_KALMANBOXTRACKER_H_
#define IOU_BASED_MOT_KALMANBOXTRACKER_H_

#include "iou_based_mot/dataProcess.h"
#include "iou_based_mot/kalman.hpp"
#include <memory>
#include <vector>

namespace hobot {
namespace iou_mot {
class KalmanBoxtracker {
 public:
  KalmanBoxtracker() = default;

  explicit KalmanBoxtracker(const sp_BBox &bbox);

  ~KalmanBoxtracker() = default;

  int update(const sp_BBox &bbox);

  sp_BBox predict(int width, int height);

  sp_BBox get_state();

  spKalmanFilter m_pKf;

 private:
  int m_age = 0;
  std::vector<sp_BBox> m_history;
};

typedef std::shared_ptr<KalmanBoxtracker> spKalmanBoxTracker;
}  // namespace iou_mot
}  // namespace hobot

#endif  // IOU_BASED_MOT_KALMANBOXTRACKER_H_
