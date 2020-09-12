/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: SquatEvent.h
 * @Brief: declaration of the SquatEvent
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-05-27 9:18:28
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-05-27 16:01:54
 */

#ifndef BEHAVIORMETHOD_SQUATEVENT_H_
#define BEHAVIORMETHOD_SQUATEVENT_H_

#include <vector>
#include "BehaviorMethod/BehaviorEvent.h"
#include "horizon/vision_type/vision_type.hpp"

namespace xstream {

class SquatEvent : public BehaviorEvent {
 public:
  SquatEvent() {}
  virtual ~SquatEvent() {}

  int Init(const Json::Value &config) override;

  bool IsEvent(hobot::vision::Landmarks kps) override;

 private:
  float differ_ = 1.3f;  // 肩胯、膝胯比例
  float differ_2_ = 1.0f;  // 肩胯、脚胯比例
  float upper_body_ = 30.0f;  // 上半身倾斜角度
  float upper_body_2_ = 60.0f;  // 上半身后倾角度
  float angle_ = 130.0f;  // 大腿、小腿夹角
  float skeleton_score_thres_ = 0.2f;
};

}  // namespace xstream

#endif  // BEHAVIORMETHOD_SQUATEVENT_H_
