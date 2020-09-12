//
// Created by kai01.han on 6/1/19.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#ifndef IOU_BASED_MOT_MATCHER_H
#define IOU_BASED_MOT_MATCHER_H

#include <vector>
#include "iou_based_mot/target.h"
#include "iou_based_mot/utility.h"
#include "iou_based_mot/tracklet.h"

namespace hobot {
namespace iou_mot {

class Matcher {
 public:
    Matcher() = default;
 private:
  const int img_width = 1920;
  const int img_height = 1080;
};

}  // namespace iou_mot
}  // namespace hobot

#endif //IOU_BASED_MOT_MATCHER_H
