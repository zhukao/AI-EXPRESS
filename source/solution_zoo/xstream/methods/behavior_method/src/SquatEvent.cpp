/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: SquatEvent.cpp
 * @Brief: definition of the SquatEvent
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-05-25 14:27:05
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-05-25 16:17:08
 */

#include "BehaviorMethod/SquatEvent.h"
#include <vector>
#include <memory>
#include <algorithm>
#include "hobotlog/hobotlog.hpp"
#include "horizon/vision_type/vision_type.hpp"

namespace xstream {

int SquatEvent::Init(const Json::Value &config) {
  BehaviorEvent::Init(config);

  // 肩胯、膝胯比例
  if (config.isMember("body_knee_ratio")) {
    differ_ = config["body_knee_ratio"].asFloat();
    HOBOT_CHECK(differ_ > 0);
  }

  // 肩胯、脚胯比例
  if (config.isMember("body_feet_ratio")) {
    differ_2_ = config["body_feet_ratio"].asFloat();
    HOBOT_CHECK(differ_2_ > 0);
  }

  // 上半身倾斜角度
  if (config.isMember("body_slope")) {
    upper_body_ = config["body_slope"].asFloat();
    HOBOT_CHECK(upper_body_ > 0);
  }

  // 大腿、小腿夹角
  if (config.isMember("knee_bending_angle")) {
    angle_ = config["knee_bending_angle"].asFloat();
    HOBOT_CHECK(angle_ > 0);
  }

  // 骨骼点阈值
  if (config.isMember("skeleton_score_thres")) {
    skeleton_score_thres_ = config["skeleton_score_thres"].asFloat();
    HOBOT_CHECK(skeleton_score_thres_ > 0);
  }
  return 0;
}

bool SquatEvent::IsEvent(hobot::vision::Landmarks kps) {
  // 下蹲导致躯体重叠，kps score较低，取消对下半身score的判断
  if (kps.values[5].score < skeleton_score_thres_ ||
      kps.values[6].score < skeleton_score_thres_) {
    return false;
  }

  // 肩中部
  auto center_56_x = (kps.values[5].x + kps.values[6].x) / 2;
  auto center_56_y = (kps.values[5].y + kps.values[6].y) / 2;
  auto center_56 = hobot::vision::Point(center_56_x, center_56_y);
  // 胯中部
  auto center_1112_x = (kps.values[11].x + kps.values[12].x) / 2;
  auto center_1112_y = (kps.values[11].y + kps.values[12].y) / 2;
  auto center_1112 = hobot::vision::Point(center_1112_x, center_1112_y);
  // 膝中部
  auto center_1314_x = (kps.values[13].x + kps.values[14].x) / 2;
  auto center_1314_y = (kps.values[13].y + kps.values[14].y) / 2;
  auto center_1314 = hobot::vision::Point(center_1314_x, center_1314_y);
  // 脚中部
  auto center_1516_x = (kps.values[15].x + kps.values[16].x) / 2;
  auto center_1516_y = (kps.values[15].y + kps.values[16].y) / 2;
  auto center_1516 = hobot::vision::Point(center_1516_x, center_1516_y);

  // 判断上半身是否倒地
  auto a = center_56.y - center_1112.y;
  auto b = center_56.x - center_1112.x;
  auto upbody_slope = CalculateSlope(a, b);
  if (upbody_slope <= upper_body_) {
    return false;
  }

  // 判断是否是躺倒
  a = std::max(center_56.x, center_1314.x);
  b = std::min(center_56.x, center_1314.x);
  if (!(center_1112.x <= a && center_1112.x >= b) &&
      upbody_slope < upper_body_2_) {
    return false;
  }

  // 判断屈膝
  {
    float angle_1 = CalculateAngle(kps.values[14],
                                   kps.values[12],
                                   kps.values[16]);
    float angle_2 = CalculateAngle(kps.values[13],
                                   kps.values[11],
                                   kps.values[15]);
    if (angle_1 > angle_ && angle_2 > angle_) {
      return false;
    }
  }

  // 判断肩胯、脚胯
  a = std::abs(center_56.y - center_1112.y);  // 肩胯差
  b = std::abs(center_1516.y - center_1112.y);  // 脚胯差
  float epsilon = 0.0001;

  if (b > epsilon && a/b < differ_2_) {
    return false;
  }

  // 判断肩胯、膝胯
  a = std::abs(center_56.y - center_1112.y);  // 肩胯差
  b = std::abs(center_1314.y - center_1112.y);  // 膝胯差

  if (b < epsilon || a/b > differ_) {
    return true;
  }

  return false;
}

}  // namespace xstream
