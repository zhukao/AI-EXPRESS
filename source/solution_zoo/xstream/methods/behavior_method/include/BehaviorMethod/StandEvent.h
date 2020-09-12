/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: StandEvent.h
 * @Brief: declaration of the StandEvent
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-05-27 9:18:28
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-05-27 16:01:54
 */

#ifndef BEHAVIORMETHOD_STANDEVENT_H_
#define BEHAVIORMETHOD_STANDEVENT_H_

#include <vector>
#include "BehaviorMethod/BehaviorEvent.h"
#include "horizon/vision_type/vision_type.hpp"

namespace xstream {

class StandEvent : public BehaviorEvent {
 public:
  StandEvent() {}
  virtual ~StandEvent() {}

  int Init(const Json::Value &config) override;

  bool IsEvent(hobot::vision::Landmarks kps) override;

 private:
  float slope_ = 75.0f;
  float angle_ = 170.0f;
  float skeleton_score_thres_ = 0.2f;
};

}  // namespace xstream

#endif  // BEHAVIORMETHOD_STANDEVENT_H_
