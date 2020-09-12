/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: BehaviorFactory.h
 * @Brief: declaration of the BehaviorFactory
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-05-25 14:18:28
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-05-25 16:01:54
 */

#ifndef BEHAVIORMETHOD_BEHAVIORFACTORY_H_
#define BEHAVIORMETHOD_BEHAVIORFACTORY_H_

#include "BehaviorMethod/BehaviorEvent.h"
#include "BehaviorMethod/RaiseHandEvent.h"
#include "BehaviorMethod/StandEvent.h"
#include "BehaviorMethod/SquatEvent.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

class BehaviorFactory {
 public:
  static BehaviorEvent* GetPredictor(Behavior behavior) {
    switch (behavior) {
      case Behavior::RAISE_HAND:
        return new RaiseHandEvent();
      case Behavior::STAND:
        return new StandEvent();
      case Behavior::SQUAT:
        return new SquatEvent();
      default: {
         HOBOT_CHECK(false) << "Invalid behavior type";
         return nullptr;
      }
    }
    return nullptr;
  }
};

}  // namespace xstream
#endif  // BEHAVIORMETHOD_BEHAVIORFACTORY_H_
