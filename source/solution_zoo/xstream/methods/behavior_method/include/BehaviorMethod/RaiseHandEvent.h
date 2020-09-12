/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: RaiseHandEvent.h
 * @Brief: declaration of the RaiseHandEvent
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-05-25 14:18:28
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-05-25 16:01:54
 */

#ifndef BEHAVIORMETHOD_RAISEHANDEVENT_H_
#define BEHAVIORMETHOD_RAISEHANDEVENT_H_

#include <vector>
#include "BehaviorMethod/BehaviorEvent.h"
#include "horizon/vision_type/vision_type.hpp"

namespace xstream {

class RaiseHandEvent : public BehaviorEvent {
 public:
  RaiseHandEvent() {}
  virtual ~RaiseHandEvent() {}

  int Init(const Json::Value &config) override;

  bool IsEvent(hobot::vision::Landmarks kps) override;

 private:
  float tan_threshold_ = 40.0f;
  float skeleton_score_thres_ = 0.2f;
};

}  // namespace xstream

#endif  // BEHAVIORMETHOD_RAISEHANDEVENT_H_
