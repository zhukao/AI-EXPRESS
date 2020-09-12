/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: StandEvent.cpp
 * @Brief: definition of the StandEvent
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-05-25 14:27:05
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-05-25 16:17:08
 */

#include "BehaviorMethod/StandEvent.h"
#include <vector>
#include <memory>
#include "hobotlog/hobotlog.hpp"
#include "horizon/vision_type/vision_type.hpp"

namespace xstream {

int StandEvent::Init(const Json::Value &config) {
  BehaviorEvent::Init(config);

  // 身体倾斜角度
  if (config.isMember("body_slope")) {
    slope_ = config["body_slope"].asFloat();
    HOBOT_CHECK(slope_ > 0);
  }

  // 肢体夹角
  if (config.isMember("body_angle")) {
    angle_ = config["body_angle"].asFloat();
    HOBOT_CHECK(angle_ > 0);
  }

  // 骨骼点阈值
  if (config.isMember("skeleton_score_thres")) {
    skeleton_score_thres_ = config["skeleton_score_thres"].asFloat();
    HOBOT_CHECK(skeleton_score_thres_ > 0);
  }
  return 0;
}

bool StandEvent::IsEvent(hobot::vision::Landmarks kps) {
  if (kps.values[5].score < skeleton_score_thres_ ||
      kps.values[6].score < skeleton_score_thres_ ||  // 肩
      kps.values[11].score < skeleton_score_thres_ ||
      kps.values[12].score < skeleton_score_thres_ ||  // 胯
      kps.values[13].score < skeleton_score_thres_ ||
      kps.values[14].score < skeleton_score_thres_ ||  // 膝
      kps.values[15].score < skeleton_score_thres_ ||
      kps.values[16].score < skeleton_score_thres_) {  // 脚
    return false;
  }
  auto center_56_x = (kps.values[5].x + kps.values[6].x) / 2;
  auto center_56_y = (kps.values[5].y + kps.values[6].y) / 2;
  auto center_56 = hobot::vision::Point(center_56_x, center_56_y);

  auto center_1112_x = (kps.values[11].x + kps.values[12].x) / 2;
  auto center_1112_y = (kps.values[11].y + kps.values[12].y) / 2;
  auto center_1112 = hobot::vision::Point(center_1112_x, center_1112_y);

  auto center_1516_x = (kps.values[15].x + kps.values[16].x) / 2;
  auto center_1516_y = (kps.values[15].y + kps.values[16].y) / 2;
  auto center_1516 = hobot::vision::Point(center_1516_x, center_1516_y);

  // 上、下身与地面夹角过小
  float up_body_slope, low_body_slope;
  {
    auto a = center_56.y - center_1112.y;
    auto b = center_56.x - center_1112.x;
    up_body_slope = CalculateSlope(a, b);
  }
  {
    auto a = center_1516.y - center_1112.y;
    auto b = center_1516.x - center_1112.x;
    low_body_slope = CalculateSlope(a, b);
  }
  if (up_body_slope < slope_ || low_body_slope < slope_) {
    return false;
  }

  // 肩胯、脚胯的角度
  {
    float angle_1 = CalculateAngle(kps.values[12],
                                   kps.values[6],
                                   kps.values[16]);
    float angle_2 = CalculateAngle(kps.values[11],
                                   kps.values[5],
                                   kps.values[15]);
    if (angle_1 < angle_ && angle_2 < angle_) {
      return false;
    }
  }

  // 膝胯、膝脚的角度
  {
    float angle_1 = CalculateAngle(kps.values[13],
                                   kps.values[11],
                                   kps.values[15]);
    float angle_2 = CalculateAngle(kps.values[14],
                                   kps.values[12],
                                   kps.values[16]);
    if (angle_1 < angle_ || angle_2 < angle_) {
      return false;
    }
  }

  return true;
}

}  // namespace xstream
